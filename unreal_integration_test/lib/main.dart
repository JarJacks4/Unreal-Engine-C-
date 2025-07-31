import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Lucille Home - Escape Innerverse',
      theme: ThemeData(
        primarySwatch: Colors.purple,
        visualDensity: VisualDensity.adaptivePlatformDensity,
      ),
      home: LucilleHomePage(),
    );
  }
}

class LucilleHomePage extends StatefulWidget {
  @override
  _LucilleHomePageState createState() => _LucilleHomePageState();
}

class _LucilleHomePageState extends State<LucilleHomePage> {
  static const platform = MethodChannel('unreal_bridge');
  bool _isUnrealRunning = false;

  @override
  void initState() {
    super.initState();
    _checkUnrealStatus();
  }

  Future<void> _checkUnrealStatus() async {
    try {
      final bool isRunning = await platform.invokeMethod('isUnrealRunning');
      setState(() {
        _isUnrealRunning = isRunning;
      });
    } on PlatformException catch (e) {
      print("Failed to check Unreal status: '${e.message}'.");
    }
  }

  Future<void> _launchEscapeInnerverse() async {
    try {
      final bool result = await platform.invokeMethod('launchUnreal');
      
      if (result) {
        setState(() {
          _isUnrealRunning = true;
        });
        
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Escape Innerverse launched!'),
            backgroundColor: Colors.green,
          ),
        );
      }
    } on PlatformException catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Failed to launch Escape Innerverse: ${e.message}'),
          backgroundColor: Colors.red,
        ),
      );
    }
  }

  Future<void> _launchUnrealLevel(String levelName) async {
    try {
      final bool result = await platform.invokeMethod('launchUnrealLevel', {
        'levelName': levelName,
      });
      
      if (result) {
        setState(() {
          _isUnrealRunning = true;
        });
        
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Unreal Engine launched with level: $levelName')),
        );
      }
    } on PlatformException catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Failed to launch Unreal: ${e.message}')),
      );
    }
  }

  Future<void> _stopUnreal() async {
    try {
      final bool result = await platform.invokeMethod('stopUnreal');
      
      if (result) {
        setState(() {
          _isUnrealRunning = false;
        });
        
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Unreal Engine stopped')),
        );
      }
    } on PlatformException catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Failed to stop Unreal: ${e.message}')),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Lucille Home'),
        backgroundColor: _isUnrealRunning ? Colors.green : Colors.purple,
        actions: [
          IconButton(
            icon: Icon(_isUnrealRunning ? Icons.stop : Icons.play_arrow),
            onPressed: _isUnrealRunning ? _stopUnreal : null,
          ),
        ],
      ),
      body: Container(
        decoration: BoxDecoration(
          gradient: LinearGradient(
            begin: Alignment.topCenter,
            end: Alignment.bottomCenter,
            colors: [Colors.purple[100]!, Colors.purple[300]!],
          ),
        ),
        child: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              // Lucille Avatar/Logo
              Container(
                width: 120,
                height: 120,
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  color: Colors.white,
                  boxShadow: [
                    BoxShadow(
                      color: Colors.black26,
                      blurRadius: 10,
                      offset: Offset(0, 5),
                    ),
                  ],
                ),
                child: Icon(
                  Icons.psychology,
                  size: 60,
                  color: Colors.purple[600],
                ),
              ),
              SizedBox(height: 30),
              
              // Welcome Text
              Text(
                'Welcome to Lucille',
                style: TextStyle(
                  fontSize: 28,
                  fontWeight: FontWeight.bold,
                  color: Colors.purple[800],
                ),
              ),
              SizedBox(height: 10),
              Text(
                'Your AI Wellness Companion',
                style: TextStyle(
                  fontSize: 16,
                  color: Colors.purple[600],
                ),
              ),
              SizedBox(height: 40),
              
              // Escape Innerverse Card
              GestureDetector(
                onTap: _launchEscapeInnerverse,
                child: Container(
                  width: 300,
                  height: 200,
                  decoration: BoxDecoration(
                    borderRadius: BorderRadius.circular(20),
                    gradient: LinearGradient(
                      begin: Alignment.topLeft,
                      end: Alignment.bottomRight,
                      colors: [Colors.blue[400]!, Colors.purple[600]!],
                    ),
                    boxShadow: [
                      BoxShadow(
                        color: Colors.black26,
                        blurRadius: 10,
                        offset: Offset(0, 5),
                      ),
                    ],
                  ),
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      Icon(
                        Icons.games,
                        size: 50,
                        color: Colors.white,
                      ),
                      SizedBox(height: 15),
                      Text(
                        'Escape Innerverse',
                        style: TextStyle(
                          fontSize: 24,
                          fontWeight: FontWeight.bold,
                          color: Colors.white,
                        ),
                      ),
                      SizedBox(height: 10),
                      Text(
                        'Enter the Unreal Engine Experience',
                        style: TextStyle(
                          fontSize: 14,
                          color: Colors.white70,
                        ),
                        textAlign: TextAlign.center,
                      ),
                    ],
                  ),
                ),
              ),
              SizedBox(height: 30),
              
              // Status Indicator
              Container(
                padding: EdgeInsets.symmetric(horizontal: 20, vertical: 10),
                decoration: BoxDecoration(
                  color: _isUnrealRunning ? Colors.green : Colors.grey[300],
                  borderRadius: BorderRadius.circular(20),
                ),
                child: Text(
                  _isUnrealRunning ? 'Unreal Engine Running' : 'Ready to Launch',
                  style: TextStyle(
                    color: _isUnrealRunning ? Colors.white : Colors.grey[700],
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
} 