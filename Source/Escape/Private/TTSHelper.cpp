// TTSHelper.cpp
#include "TTSHelper.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HAL/UnrealMemory.h"
#include "Sound/SoundWaveProcedural.h"
#include "AudioDevice.h"
#include "Misc/ScopeLock.h"
#include "AudioDecompress.h"
#include "Audio.h"
#include "Async/Async.h"

// Minimal WAV header parser
struct FWaveHeader
{
    uint16 NumChannels = 1;
    uint32 SampleRate = 24000;
    uint16 BitsPerSample = 16;
    int32  PCMDataStart = 0;
    int32  PCMDataSize = 0;
};

// Parse a simple PCM WAV (RIFF) to find data chunk
static bool ParseWav(const TArray<uint8>& Bytes, FWaveHeader& Out, const uint8*& PCM, int32& PCMBytes)
{
    if (Bytes.Num() < 44) return false;
    const uint8* D = Bytes.GetData();

    auto Read16 = [&](int32 Off) { return *(uint16*)(D + Off); };
    auto Read32 = [&](int32 Off) { return *(uint32*)(D + Off); };

    if (FMemory::Memcmp(D, "RIFF", 4) != 0) return false;
    if (FMemory::Memcmp(D + 8, "WAVE", 4) != 0) return false;

    // fmt chunk should start around 12
    int32 Offset = 12;
    while (Offset + 8 <= Bytes.Num())
    {
        const uint32 ChunkId = Read32(Offset);
        const uint32 ChunkSz = Read32(Offset + 4);
        const uint8* Chunk = D + Offset + 8;

        if (ChunkId == *(uint32*)"fmt ")
        {
            if (ChunkSz < 16) return false;
            const uint16 AudioFormat = *(uint16*)(Chunk + 0);
            Out.NumChannels = *(uint16*)(Chunk + 2);
            Out.SampleRate = *(uint32*)(Chunk + 4);
            Out.BitsPerSample = *(uint16*)(Chunk + 14);
            if (AudioFormat != 1) return false; // PCM only
        }
        else if (ChunkId == *(uint32*)"data")
        {
            Out.PCMDataStart = Offset + 8;
            Out.PCMDataSize = ChunkSz;
            PCM = D + Out.PCMDataStart;
            PCMBytes = Out.PCMDataSize;
            return true;
        }

        Offset += 8 + ChunkSz;
    }
    return false;
}

static USoundWaveProcedural* CreateProceduralWave(UObject* Outer, uint32 SampleRate, uint16 NumChannels)
{
    USoundWaveProcedural* SW = NewObject<USoundWaveProcedural>(Outer);
    SW->bLooping = false;
    SW->NumChannels = NumChannels;
    SW->Duration = INDEFINITELY_LOOPING_DURATION;

    SW->SetSampleRate((int32)SampleRate);

    return SW;
}

static bool IsEmojiCodepoint(uint32 CP)
{
    if ((CP >= 0x1F300 && CP <= 0x1FAFF) ||   // pictographs
        (CP >= 0x1F1E6 && CP <= 0x1F1FF) ||   // flags
        (CP >= 0x2600 && CP <= 0x26FF) ||   // misc symbols
        (CP >= 0x2700 && CP <= 0x27BF) ||   // dingbats
        (CP == 0xFE0F) || (CP == 0x200D))     // variation selector / ZWJ
    {
        return true;
    }
    return false;
}

static FString XMLEscape(const FString& In)
{
    FString S = In;
    // Order matters: ampersand first to avoid double-escaping
    S.ReplaceInline(TEXT("&"), TEXT("&amp;"));
    S.ReplaceInline(TEXT("<"), TEXT("&lt;"));
    S.ReplaceInline(TEXT(">"), TEXT("&gt;"));
    S.ReplaceInline(TEXT("\""), TEXT("&quot;"));
    S.ReplaceInline(TEXT("'"), TEXT("&apos;"));
    return S;
}

static FString NormalizeForSSML(const FString& In, const TCHAR* BreakMs = TEXT("250ms"))
{
    static const TCHAR* BR = TEXT("§§BR§§");

    FString S = In;

    // Collapse double backslashes first (handles \\\' -> \' cases)
    while (S.Contains(TEXT("\\\\"))) { S.ReplaceInline(TEXT("\\\\"), TEXT("\\")); }

    // Un-escape common quote escapes
    S.ReplaceInline(TEXT("\\'"), TEXT("'"));
    S.ReplaceInline(TEXT("\\\""), TEXT("\""));
    S.ReplaceInline(TEXT("\\u2019"), TEXT("'")); // JSON unicode apostrophe

    // Map line breaks/tabs to placeholder *after* un-escapes
    S.ReplaceInline(TEXT("\\r\\n"), BR);
    S.ReplaceInline(TEXT("\\n"), BR);
    S.ReplaceInline(TEXT("\r\n"), BR);
    S.ReplaceInline(TEXT("\n"), BR);
    S.ReplaceInline(TEXT("\\t"), TEXT(" "));
    S.ReplaceInline(TEXT("\t"), TEXT(" "));

    // Normalize curly quotes
    S.ReplaceInline(TEXT("’"), TEXT("'"));
    S.ReplaceInline(TEXT("‘"), TEXT("'"));
    S.ReplaceInline(TEXT("“"), TEXT("\""));
    S.ReplaceInline(TEXT("”"), TEXT("\""));

    // HARD purge any remaining backslashes (ensures Azure can't say "backslash")
    {
        FString NoSlash; NoSlash.Reserve(S.Len());
        for (int32 i = 0; i < S.Len(); ++i)
        {
            const TCHAR ch = S[i];
            if (ch != TEXT('\\')) { NoSlash.AppendChar(ch); }
        }
        S = MoveTemp(NoSlash);
    }

    // ------------- Emoji strip (UTF-16 aware) -------------
    FString NoEmoji; NoEmoji.Reserve(S.Len());
    const TCHAR* Data = *S; const int32 Len = S.Len();
    for (int32 i = 0; i < Len; ++i)
    {
        uint32 CP = (uint16)Data[i];
        if (CP >= 0xD800 && CP <= 0xDBFF && (i + 1) < Len)
        {
            uint32 High = CP, Low = (uint16)Data[i + 1];
            if (Low >= 0xDC00 && Low <= 0xDFFF)
            {
                CP = 0x10000 + (((High - 0xD800) << 10) | (Low - 0xDC00));
                if (!IsEmojiCodepoint(CP)) { NoEmoji.AppendChar((TCHAR)High); NoEmoji.AppendChar((TCHAR)Low); }
                i++; continue;
            }
        }
        else
        {
            if (IsEmojiCodepoint(CP)) { continue; }
        }
        // Drop other control chars (keep space)
        if (CP < 0x20 && CP != '\t' && CP != '\n' && CP != '\r' && CP != ' ') { continue; }
        NoEmoji.AppendChar((TCHAR)CP);
    }

    // Collapse spaces
    while (NoEmoji.Contains(TEXT("  "))) { NoEmoji.ReplaceInline(TEXT("  "), TEXT(" ")); }

    NoEmoji.ReplaceInline(TEXT("**"), TEXT(""));

    // Escape XML special chars
    FString Escaped = XMLEscape(NoEmoji);

    // Placeholder -> real break
    const FString BreakTag = FString::Printf(TEXT("<break time=\"%s\"/>"), BreakMs);
    Escaped.ReplaceInline(BR, *BreakTag);

    return Escaped;
}


void UTTSHelper::SynthesizeAndPlay_Azure(
    const FString& AzureKey,
    const FString& AzureRegion,
    const FString& VoiceName,
    const FString& Text,
    UAudioComponent* AudioComp)
{
    if (!AudioComp) { UE_LOG(LogTemp, Warning, TEXT("TTS: AudioComponent is null.")); return; }
    if (Text.IsEmpty()) { UE_LOG(LogTemp, Warning, TEXT("TTS: Empty text.")); return; }

    const FString Url = FString::Printf(TEXT("https://%s.tts.speech.microsoft.com/cognitiveservices/v1"), *AzureRegion);

    const FString Escaped = NormalizeForSSML(Text, TEXT("250ms"));
    FString SSML = FString::Printf(
        TEXT("<speak version='1.0' xml:lang='en-US'><voice name='%s'>%s</voice></speak>"),
        *VoiceName, *Escaped);

    auto Req = FHttpModule::Get().CreateRequest();
    Req->SetVerb(TEXT("POST"));
    Req->SetURL(Url);
    Req->SetHeader(TEXT("Ocp-Apim-Subscription-Key"), AzureKey);
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/ssml+xml"));
    Req->SetHeader(TEXT("X-Microsoft-OutputFormat"), TEXT("riff-48khz-16bit-mono-pcm")); // 48k mono PCM
    Req->SetHeader(TEXT("User-Agent"), TEXT("UE5-Lucia-TTS"));
    Req->SetContentAsString(SSML);

    TWeakObjectPtr<UAudioComponent> WeakAudio = AudioComp;

    Req->OnProcessRequestComplete().BindLambda(
        [WeakAudio](FHttpRequestPtr, FHttpResponsePtr Resp, bool bOK)
        {
            if (!bOK || !Resp.IsValid()) { UE_LOG(LogTemp, Error, TEXT("TTS: HTTP failed")); return; }
            const int32 Code = Resp->GetResponseCode();
            if (Code < 200 || Code >= 300) { UE_LOG(LogTemp, Error, TEXT("TTS: HTTP %d — %s"), Code, *Resp->GetContentAsString()); return; }

            const TArray<uint8>& Bytes = Resp->GetContent();
            FWaveHeader H; const uint8* PCM = nullptr; int32 PCMBytes = 0;
            if (!ParseWav(Bytes, H, PCM, PCMBytes) || !PCM || PCMBytes <= 0) { UE_LOG(LogTemp, Error, TEXT("TTS: parse WAV failed")); return; }

            // Copy PCM into a UE array so we can pass it on the game thread
            TArray<uint8> PCMArray;
            PCMArray.Append(PCM, PCMBytes);

            // Build sound + play, and feed the driver, on the game thread
            AsyncTask(ENamedThreads::GameThread, [WeakAudio, PCMArray = MoveTemp(PCMArray), SampleRate = (int32)H.SampleRate]()
                {
                    if (WeakAudio.IsValid())
                    {
                        UObject* Outer = WeakAudio.Get();
                        USoundWaveProcedural* SW = NewObject<USoundWaveProcedural>(Outer);
                        SW->SetSampleRate(SampleRate);
                        SW->NumChannels = 1;
                        SW->Duration = INDEFINITELY_LOOPING_DURATION;
                        SW->bLooping = false;
                        SW->SoundGroup = SOUNDGROUP_Voice;

                        SW->QueueAudio(PCMArray.GetData(), PCMArray.Num());

                        WeakAudio->SetSound(SW);
                        WeakAudio->Play(0.f);
                    }
                });
        });

    Req->ProcessRequest();
}

