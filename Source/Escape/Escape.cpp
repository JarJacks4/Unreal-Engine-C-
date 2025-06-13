// Copyright Epic Games, Inc. All Rights Reserved.

#include "Escape.h" // Include the header file for the module itself
#include "Modules/ModuleManager.h" // Required for implementing a game module

/**
 *  Implements the primary game module for the Escape project.
 * This macro is essential for Unreal Engine to recognize and load the game module.
 * - FDefaultGameModuleImpl: Specifies the use of the standard game module implementation provided by the engine.
 * - Escape: The C++ class name of the module (should match the module name).
 * - "Escape": The string name of the module, used for identification by the engine and build tools.
 */
IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, Escape, "Escape" );
