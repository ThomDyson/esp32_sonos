/************************************************************************/
/* MicroXPath lib, an XML navigator with a tiny memory footprint, v1.2. */
/*                                                                      */
/* This library is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This library is distributed in the hope that it will be useful, but  */
/* WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     */
/* General Public License for more details.                             */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this library. If not, see <http://www.gnu.org/licenses/>. */
/*                                                                      */
/* Written by Thomas Mittet (code@lookout.no) January 2014.             */
/************************************************************************/

#ifndef MicroXPath_P_h
#define MicroXPath_P_h

// When pico mode is enabled, XML validation and error handling is disabled
#define XML_PICO_MODE 1

#include <Arduino.h>
#if (defined(__AVR__))
  #include <avr/pgmspace.h>
#else
  #include <pgmspace.h>
#endif

#define XML_PARSER_UNINITIATED 0
#define XML_PARSER_ROOT 1
#define XML_PARSER_START_TAG 2
#define XML_PARSER_START_TAG_NAME 3
#define XML_PARSER_PROLOG_TAG 4
#define XML_PARSER_PROLOG_TAG_NAME 5
#define XML_PARSER_PROLOG_END 6
#define XML_PARSER_PROLOG_ATTRIBUTES 7
#define XML_PARSER_PROLOG_ATTRIBUTE_VALUE 8
#define XML_PARSER_ATTRIBUTES 9
#define XML_PARSER_ATTRIBUTE_VALUE 10
#define XML_PARSER_ELEMENT_CONTENT 11
#define XML_PARSER_COMMENT 12
#define XML_PARSER_END_TAG 13
#define XML_PARSER_COMPLETE 14

#ifndef XML_PICO_MODE

#define XML_PROLOG "xml"

#define XML_ERROR_MALFORMED_PROLOG 1
#define XML_ERROR_MALFORMED_START_TAG 1 << 1
#define XML_ERROR_TAG_START_INSIDE_TAG 1 << 2
#define XML_ERROR_MISSPLACED_END_CHAR 1 << 3
#define XML_ERROR_MALFORMED_END_TAG 1 << 4
#define XML_ERROR_TAG_END_OUTSIDE_TAG 1 << 5
#define XML_ERROR_NOT_WELL_FORMED 1 << 6

#endif

class MicroXPath_P {
  public:
    uint8_t state;
    uint8_t oldstate;
    #ifndef XML_PICO_MODE
    uint8_t errors;
    #endif
    
    MicroXPath_P();

    void reset();
    #ifndef XML_PICO_MODE
    bool validate(char charToParse);
    #endif
    void setPath(PGM_P *path, size_t pathSize);
    bool findValue(char charToParse);
    bool getValue(char charToParse, char *result, size_t resultSize);

  private:
    PGM_P *path;
    size_t pathSize;
    #ifndef XML_PICO_MODE
    uint8_t tagChecksum;
    uint8_t checksum;
    #endif
    uint8_t level;
    uint16_t position;
    uint16_t matchCount;
    uint8_t matchLevel;

    bool find(char charToParse /* , const char *attributeName, const char *attributeValue */);
    bool elementPathMatch();
};

#endif
