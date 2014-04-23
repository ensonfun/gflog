/********************************************************************
	created:	2013/07/17
	created:	17:7:2013   15:18
	author:		高峰

	purpose:	从google移植过来，去掉无用代码 + 符合本地编程习惯

  用法：在main函数中：CommandLine::Init()
        在任何需要使用的地方CommandLine::ForCurrentProcess()
        singleton用法:-D
*********************************************************************/

// Google的描述 ;-)
// This class works with command lines: building and parsing.
// Arguments with prefixes ('--', '-', and on Windows, '/') are switches.
// Switches will precede all other arguments without switch prefixes.
// Switches can optionally have values, delimited by '=', e.g., "-switch=value".
// An argument of "--" will terminate switch parsing during initialization,
// interpreting subsequent tokens as non-switch arguments, regardless of prefix.

// There is a singleton read-only CommandLine that represents the command line
// that the current process was started with.  It must be initialized in main().

#pragma once
#include <string>
#include <vector>
#include <map>

class CommandLine
{
public:
  typedef std::vector<std::wstring> StringVector;
  typedef std::map<std::wstring, std::wstring> SwitchMap;

  // A constructor for CommandLines that only carry switches and arguments.
  enum NoProgram { NO_PROGRAM };
  explicit CommandLine(NoProgram no_program);

  // Construct a new command line from an argument list.
  CommandLine(int argc, const wchar_t* const* argv);
  explicit CommandLine(const StringVector& argv);

  ~CommandLine();

  // Initialize the current process CommandLine singleton. On Windows, ignores
  // its arguments (we instead parse GetCommandLineW() directly) because we
  // don't trust the CRT's parsing of the command line, but it still must be
  // called to set up the command line. Returns false if initialization has
  // already occurred, and true otherwise. Only the caller receiving a 'true'
  // return value should take responsibility for calling Reset.
  static bool Init();

  // Destroys the current process CommandLine singleton. This is necessary if
  // you want to reset the base library to its initial state (for example, in an
  // outer library that needs to be able to terminate, and be re-initialized).
  // If Init is called only once, as in main(), Reset() is not necessary.
  static void Reset();

  // Get the singleton CommandLine representing the current process's
  // command line. Note: returned value is mutable, but not thread safe;
  // only mutate if you know what you're doing!
  static CommandLine* ForCurrentProcess();

  static CommandLine FromString(const std::wstring& command_line);

  void InitFromArgv(int argc, const wchar_t* const* argv);
  void InitFromArgv(const StringVector& argv);
  void InitFromString(const std::wstring& command_line);
  void SetProgram(const std::wstring& program);
  void AppendSwitchNative(const std::wstring& switch_string,
    const std::wstring& value);
  void AppendArgNative(const std::wstring& value);
  std::wstring GetProgram() const;
  std::wstring GetSwitchValueNative(
    const std::wstring& switch_string) const;
  std::wstring GetArgumentsString() const;
  bool HasSwitch(const std::wstring& switch_string) const;
  void ParseFromString(const std::wstring& command_line);

private:
  CommandLine(void);

  static CommandLine* m_current_process_commandline;

  StringVector m_vecArgs;
  SwitchMap m_mapSwitch;
  size_t m_nBeginArgs;
};
