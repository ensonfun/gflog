#include "command_line.h"
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <windows.h>
#include <shellapi.h>
#include "comm/logging.h"
#include <atlstr.h>

CommandLine* CommandLine::m_current_process_commandline = NULL;

namespace
{
    const std::wstring kSwitchTerminator = L"--";
    const std::wstring kSwitchValueSeparator = L"=";

    const wchar_t* const kSwitchPrefixes[] = {L"--", L"-", L"/"};

    std::wstring TrimWhitespace(const std::wstring &input)
    {
        CStringW output(input.c_str());
        return std::wstring(output.Trim());

        //中文下有些问题，暂时用CString的trim
        /*  std::wstring output(input);
        output.erase(output.begin(),
        std::find_if(output.begin(), output.end(),
        std::not1(std::ptr_fun<int, int>(std::isspace)))
        );

        output.erase(std::find_if(output.rbegin(), output.rend(),
        std::not1(std::ptr_fun<int, int>(std::isspace))).base(), output.end());*/

        //return output;
    }

    size_t GetSwitchPrefixLength(const std::wstring& string)
    {
        for (size_t i = 0; i < ARRAYSIZE(kSwitchPrefixes); ++i)
        {
            std::wstring prefix(kSwitchPrefixes[i]);
            if (string.compare(0, prefix.length(), prefix) == 0)
                return prefix.length();
        }
        return 0;
    }

    bool IsSwitch(const std::wstring& string,
        std::wstring* switch_string,
        std::wstring* switch_value)
    {
        switch_string->clear();
        switch_value->clear();
        size_t prefix_length = GetSwitchPrefixLength(string);
        if (prefix_length == 0 || prefix_length == string.length())
            return false;

        const size_t equals_position = string.find(kSwitchValueSeparator);
        *switch_string = string.substr(0, equals_position);
        if (equals_position != std::wstring::npos)
            *switch_value = string.substr(equals_position + 1);
        return true;
    }

    void AppendSwitchesAndArguments(CommandLine& command_line,
        const CommandLine::StringVector& argv)
    {
        bool parse_switches = true;
        for (size_t i = 1; i < argv.size(); ++i)
        {
            std::wstring arg = argv[i];
            arg = TrimWhitespace(arg);

            std::wstring switch_string;
            std::wstring switch_value;
            parse_switches &= (arg != kSwitchTerminator);
            if (parse_switches && IsSwitch(arg, &switch_string, &switch_value))
            {
                command_line.AppendSwitchNative(switch_string, switch_value);
            }
            else
            {
                command_line.AppendArgNative(arg);
            }
        }
    }

    // Quote a string as necessary for CommandLineToArgvW compatiblity *on Windows*.
    std::wstring QuoteForCommandLineToArgvW(const std::wstring& arg)
    {
        // We follow the quoting rules of CommandLineToArgvW.
        // http://msdn.microsoft.com/en-us/library/17w5ykft.aspx
        if (arg.find_first_of(L" \\\"") == std::wstring::npos) {
            // No quoting necessary.
            return arg;
        }

        std::wstring out;
        out.push_back(L'"');
        for (size_t i = 0; i < arg.size(); ++i)
        {
            if (arg[i] == '\\')
            {
                // Find the extent of this run of backslashes.
                size_t start = i, end = start + 1;
                for (; end < arg.size() && arg[end] == '\\'; ++end)
                    /* empty */;
                size_t backslash_count = end - start;

                // Backslashes are escapes only if the run is followed by a double quote.
                // Since we also will end the string with a double quote, we escape for
                // either a double quote or the end of the string.
                if (end == arg.size() || arg[end] == '"')
                {
                    // To quote, we need to output 2x as many backslashes.
                    backslash_count *= 2;
                }
                for (size_t j = 0; j < backslash_count; ++j)
                    out.push_back('\\');

                // Advance i to one before the end to balance i++ in loop.
                i = end - 1;
            }
            else if (arg[i] == '"')
            {
                out.push_back('\\');
                out.push_back('"');
            }
            else
            {
                out.push_back(arg[i]);
            }
        }
        out.push_back('"');

        return out;
    }
} //namespace

CommandLine::CommandLine(NoProgram no_program)
: m_vecArgs(1),
m_nBeginArgs(1)
{
}

CommandLine::CommandLine(int argc, const wchar_t* const* argv)
: m_vecArgs(1),
m_nBeginArgs(1)
{
    InitFromArgv(argc, argv);
}

CommandLine::CommandLine(const StringVector& argv)
: m_vecArgs(1),
m_nBeginArgs(1)
{
    InitFromArgv(argv);
}

CommandLine::~CommandLine()
{
}

// static
bool CommandLine::Init()
{
    if (m_current_process_commandline)
    {
        // If this is intentional, Reset() must be called first. If we are using
        // the shared build mode, we have to share a single object across multiple
        // shared libraries.
        return false;
    }

    m_current_process_commandline = new CommandLine(NO_PROGRAM);
    m_current_process_commandline->ParseFromString(::GetCommandLineW());
    return true;
}

// static
void CommandLine::Reset()
{
    RSCHECK(m_current_process_commandline);
    delete m_current_process_commandline;
    m_current_process_commandline = NULL;
}

// static
CommandLine* CommandLine::ForCurrentProcess()
{
    RSCHECK(m_current_process_commandline);
    return m_current_process_commandline;
}

void CommandLine::InitFromString(const std::wstring& command_line)
{
    std::wstring command_line_string;
    command_line_string = TrimWhitespace(command_line);
    if (command_line_string.empty())
    {
        return;
    }

    int num_args = 0;
    wchar_t** args = NULL;
    args = ::CommandLineToArgvW(command_line_string.c_str(), &num_args);

    RSLOG_IF(!args,
        L"CommandLineToArgvW failed on command line: %s.", command_line.c_str());

    InitFromArgv(num_args, args);
    LocalFree(args);
}

void CommandLine::InitFromArgv(int argc,
                               const wchar_t* const* argv)
{
    StringVector new_argv;
    for (int i = 0; i < argc; ++i)
    {
        new_argv.push_back(argv[i]);
    }

    InitFromArgv(new_argv);
}

void CommandLine::InitFromArgv(const StringVector& argv)
{
    m_vecArgs = StringVector(1);
    m_nBeginArgs = 1;
    SetProgram(argv.empty() ? L"" : argv[0]);
    AppendSwitchesAndArguments(*this, argv);
}

void CommandLine::SetProgram(const std::wstring& program)
{
    m_vecArgs[0] = TrimWhitespace(program);
}

void CommandLine::AppendSwitchNative(const std::wstring& switch_string,
                                     const std::wstring& value)
{
    std::wstring switch_key(switch_string);
    std::wstring combined_switch_string(switch_key);

    size_t prefix_length = GetSwitchPrefixLength(combined_switch_string);
    m_mapSwitch[switch_key.substr(prefix_length)] = value;
    // Preserve existing switch prefixes in |m_vecArgs|; only append one if necessary.
    if (prefix_length == 0)
        combined_switch_string = kSwitchPrefixes[0] + combined_switch_string;
    if (!value.empty())
        combined_switch_string += kSwitchValueSeparator + value;
    // Append the switch and update the switches/arguments divider |m_nBeginArgs|.
    m_vecArgs.insert(m_vecArgs.begin() + m_nBeginArgs++, combined_switch_string);
}

void CommandLine::AppendArgNative(const std::wstring& value)
{
    m_vecArgs.push_back(value);
}

std::wstring CommandLine::GetProgram() const
{
    return m_vecArgs[0];
}

std::wstring CommandLine::GetSwitchValueNative(
    const std::wstring& switch_string) const
{
    SwitchMap::const_iterator result = m_mapSwitch.end();
    result = m_mapSwitch.find(switch_string);
    return result == m_mapSwitch.end() ? std::wstring() : result->second;
}

bool CommandLine::HasSwitch(const std::wstring& switch_string) const
{
    return m_mapSwitch.find(switch_string) != m_mapSwitch.end();
}

std::wstring CommandLine::GetArgumentsString() const
{
    std::wstring params;
    // Append switches and arguments.
    bool parse_switches = true;
    for (size_t i = 1; i < m_vecArgs.size(); ++i)
    {
        std::wstring arg = m_vecArgs[i];
        std::wstring switch_string;
        std::wstring switch_value;
        parse_switches &= arg != kSwitchTerminator;
        if (i > 1)
            params.append(L" ");
        if (parse_switches && IsSwitch(arg, &switch_string, &switch_value))
        {
            params.append(switch_string);
            if (!switch_value.empty())
            {
                switch_value = QuoteForCommandLineToArgvW(switch_value);
                params.append(kSwitchValueSeparator + switch_value);
            }
        }
        else
        {
            arg = QuoteForCommandLineToArgvW(arg);
            params.append(arg);
        }
    }
    return params;
}

void CommandLine::ParseFromString(const std::wstring& command_line)
{
    std::wstring command_line_string;
    command_line_string = TrimWhitespace(command_line);
    if (command_line_string.empty())
        return;

    int num_args = 0;
    wchar_t** args = NULL;
    args = ::CommandLineToArgvW(command_line_string.c_str(), &num_args);

    RSLOG_IF(!args,
        L"CommandLineToArgvW failed on command line: %s.", command_line);
    InitFromArgv(num_args, args);
    LocalFree(args);
}
