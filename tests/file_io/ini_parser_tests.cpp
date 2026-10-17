/**
 * Tests for the INI parser. Parses in-memory text, so no files or resource paths are needed.
 */

#include "moksha/shared/file_io/ini/ini_parser.h"
#include "moksha/test_framework/test_framework.h"

TEST(INIParser, SectionsAndKeys) {
    ME::INIMap iniMap = ME::INIParser::Parse(
        "[logging]\n"
        "level=info\n"
        "[network]\n"
        "port=9310\n");

    EXPECT(iniMap["logging"]["level"] == "info");
    EXPECT(iniMap["network"]["port"] == "9310");
}

TEST(INIParser, MissingKeyIsEmpty) {
    ME::INIMap iniMap = ME::INIParser::Parse("[logging]\nlevel=info\n");

    EXPECT(iniMap["logging"]["missing"].empty());
    EXPECT(iniMap["missing"]["level"].empty());
}

TEST(INIParser, WhitespaceIsTrimmed) {
    ME::INIMap iniMap = ME::INIParser::Parse(
        "  [ my section ]  \n"
        "level = info\n"
        "\tname\t=\tMoksha Engine  \n");

    EXPECT(iniMap["my section"]["level"] == "info");
    EXPECT(iniMap["my section"]["name"] == "Moksha Engine");
}

TEST(INIParser, WindowsLineEndings) {
    ME::INIMap iniMap = ME::INIParser::Parse("[settings]\r\nserverIP=127.0.0.1\r\nport=9310\r\n");

    EXPECT(iniMap["settings"]["serverIP"] == "127.0.0.1");
    EXPECT(iniMap["settings"]["port"] == "9310");
}

TEST(INIParser, LastLineWithoutNewline) {
    ME::INIMap iniMap = ME::INIParser::Parse("[settings]\nport=9310");

    EXPECT(iniMap["settings"]["port"] == "9310");
}

TEST(INIParser, CommentsAndBlankLinesSkipped) {
    ME::INIMap iniMap = ME::INIParser::Parse(
        "; comment\n"
        "\n"
        "[settings]\n"
        "   # indented comment\n"
        "port=9310\n");

    EXPECT(iniMap["settings"].size() == 1);
    EXPECT(iniMap["settings"]["port"] == "9310");
}

TEST(INIParser, ValueMayContainEquals) {
    ME::INIMap iniMap = ME::INIParser::Parse("[settings]\nquery=a=b\nempty=\n");

    EXPECT(iniMap["settings"]["query"] == "a=b");
    EXPECT(iniMap["settings"].count("empty") == 1);
    EXPECT(iniMap["settings"]["empty"].empty());
}

TEST(INIParser, MalformedLinesSkipped) {
    ME::INIMap iniMap = ME::INIParser::Parse(
        "[settings]\n"
        "no equals sign\n"
        "=no key\n"
        "[broken\n"
        "port=9310\n");

    EXPECT(iniMap["settings"].size() == 1);
    EXPECT(iniMap["settings"]["port"] == "9310");
}

TEST(INIParser, EmptySectionIsListed) {
    ME::INIMap iniMap = ME::INIParser::Parse("[empty]\n[settings]\nport=9310\n");

    EXPECT(iniMap.count("empty") == 1);
}
