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
/* UPDATE 2025 removed a parser state change that incorrectly processed */
/* attributes wit a forward slash.  Dyson                               */
/************************************************************************/

#include "MicroXPath_P.h"

#define DEBUG_PARSE_XPATH 0

MicroXPath_P::MicroXPath_P() {
  this->pathSize = 0;
  reset();
}

void MicroXPath_P::reset() {
  state = XML_PARSER_UNINITIATED;
#ifndef XML_PICO_MODE
  errors      = 0;
  tagChecksum = 0;
  checksum    = 0;
#endif
  level      = 0;
  position   = 0;
  matchCount = 0;
  matchLevel = 0;
}

#ifndef XML_PICO_MODE
bool MicroXPath_P::validate( char charToParse ) {
  if ( charToParse == 0 ) {
    return state == XML_PARSER_COMPLETE && errors == 0 && checksum == 0;
  }
  find( charToParse );
}
#endif

void MicroXPath_P::setPath( PGM_P *path, size_t pathSize ) {
  uint8_t newMatchLevel = 0;
#if DEBUG_PARSE_XPATH
  for ( uint8_t i = 0; i < pathSize; i++ ) {
    Serial.println( path[i] );
  }
#endif

  for ( uint8_t i = 0; i < pathSize && i < this->pathSize && i < matchLevel && i == newMatchLevel; i++ ) {
    uint8_t len = strlen_P( path[i] );
    if ( strlen_P( this->path[i] ) == len ) {
      for ( uint8_t j = 0; j < len; j++ ) {
        if ( pgm_read_byte( path[i] + j ) != pgm_read_byte( this->path[i] + j ) )
          break;
        if ( j == len - 1 )
          newMatchLevel++;
      }
    }
  }
  this->matchCount = 0;
  this->matchLevel = newMatchLevel;
  this->path       = path;
  this->pathSize   = pathSize;
}

bool MicroXPath_P::findValue( char charToParse ) {
  return find( charToParse ) && state == XML_PARSER_ELEMENT_CONTENT;
}

bool MicroXPath_P::getValue( char charToParse, char *result, size_t resultSize ) {
  if ( find( charToParse ) ) {
    // Ignore sub elements
    if ( level == matchLevel ) {
      if ( state == XML_PARSER_ELEMENT_CONTENT ) {
        // "position > 0" means that we skip the tag-end character and any trailing whitespace
        if ( position > 0 && matchCount < resultSize - 1 ) {
          *( result + matchCount )     = charToParse;
          *( result + matchCount + 1 ) = '\0';
          matchCount++;
        }
      } else if ( state == XML_PARSER_END_TAG && position == 0 ) {
        // Remove trailing whitespace
        char *end = result + strlen( result );
        while ( end > result && isspace( *( end - 1 ) ) )
          end--;
        *end = '\0';
        return true;
      }
    }
  }
  return false;
}

bool MicroXPath_P::find( char charToParse ) {
  // Start parsing when the first "<" character is stumbled upon
  if ( state == XML_PARSER_UNINITIATED && charToParse == '<' )
    state = XML_PARSER_ROOT;

  if ( state >= XML_PARSER_COMPLETE && charToParse > ' ' ) {
#ifndef XML_PICO_MODE
    errors |= XML_ERROR_NOT_WELL_FORMED;
#endif
  } else if ( state > XML_PARSER_UNINITIATED && state < XML_PARSER_COMPLETE ) {
    switch ( charToParse ) {
    // Tag start
    case '<':
      if ( state == XML_PARSER_ROOT || state == XML_PARSER_ELEMENT_CONTENT ) {
#ifndef XML_PICO_MODE
        tagChecksum = 0;
#endif
        state = XML_PARSER_START_TAG;
      }
#ifndef XML_PICO_MODE
      else {
        errors |= XML_ERROR_TAG_START_INSIDE_TAG;
      }
#endif
      position   = 0;
      matchCount = 0;
      break;
    // Tag end
    case '>':
      if ( state == XML_PARSER_START_TAG_NAME || state == XML_PARSER_ATTRIBUTES ) {
        if ( elementPathMatch() )
          matchLevel++;
#ifndef XML_PICO_MODE
        if ( tagChecksum )
          checksum ^= tagChecksum;
#endif
        level++;
        state = XML_PARSER_ELEMENT_CONTENT;
      } else if ( state == XML_PARSER_END_TAG ) {
        if ( level == matchLevel )
          matchLevel--;
        level--;

        state = level == 0 ? XML_PARSER_COMPLETE : XML_PARSER_ELEMENT_CONTENT;
#ifndef XML_PICO_MODE
        if ( state == XML_PARSER_COMPLETE && checksum != 0 ) {
          errors |= XML_ERROR_NOT_WELL_FORMED;
        }
#endif
      } else if ( level == 0 && state == XML_PARSER_PROLOG_END ) {
        state = XML_PARSER_ROOT;
      } else if ( state == XML_PARSER_COMMENT ) {
        state = level == 0 ? XML_PARSER_ROOT : XML_PARSER_ELEMENT_CONTENT;
      }
#ifndef XML_PICO_MODE
      else {
        if ( state == XML_PARSER_PROLOG_TAG || state == XML_PARSER_PROLOG_TAG_NAME ) {
          errors |= XML_ERROR_MALFORMED_PROLOG;
        } else if ( state == XML_PARSER_START_TAG ) {
          errors |= XML_ERROR_MALFORMED_START_TAG;
        } else {
          errors |= XML_ERROR_TAG_END_OUTSIDE_TAG;
        }
      }
#endif
      position   = 0;
      matchCount = 0;
      break;
    // Prolog start and end character
    case '?':
      if ( state == XML_PARSER_START_TAG && level == 0 ) {
        state = XML_PARSER_PROLOG_TAG;
      } else if ( state == XML_PARSER_PROLOG_TAG_NAME || state == XML_PARSER_PROLOG_ATTRIBUTES ) {
        state = XML_PARSER_PROLOG_END;
      }
      // else: Treat as content or ignore
      break;
    // Comment start character
    case '!':
      if ( state == XML_PARSER_START_TAG ) {
        state = XML_PARSER_COMMENT;
      }
      // else: Treat as content or ignore
      break;
    // Whitespace
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      switch ( state ) {
      case XML_PARSER_START_TAG_NAME:
        if ( elementPathMatch() )
          matchLevel++;
        state = XML_PARSER_ATTRIBUTES;
        break;
      case XML_PARSER_PROLOG_TAG_NAME:
#ifndef XML_PICO_MODE
        if ( position < ( sizeof( XML_PROLOG ) - 1 ) ) {
          errors |= XML_ERROR_MALFORMED_PROLOG;
        }
#endif
        state = XML_PARSER_PROLOG_ATTRIBUTES;
        break;
#ifndef XML_PICO_MODE
      case XML_PARSER_START_TAG:
        errors |= XML_ERROR_MALFORMED_START_TAG;
        break;
      case XML_PARSER_PROLOG_TAG:
      case XML_PARSER_PROLOG_END:
        errors |= XML_ERROR_MALFORMED_PROLOG;
        break;
      case XML_PARSER_END_TAG:
        errors |= XML_ERROR_MALFORMED_END_TAG;
        break;
#endif
        // default: Treat as content or ignore
      }
      break;
    // Attribute start and end character
    case '"':
    case '\'':
      switch ( state ) {
      case XML_PARSER_ATTRIBUTES:
        state = XML_PARSER_ATTRIBUTE_VALUE;
        break;
      case XML_PARSER_ATTRIBUTE_VALUE:
        state = XML_PARSER_ATTRIBUTES;
        break;
#ifndef XML_PICO_MODE
      case XML_PARSER_PROLOG_ATTRIBUTES:
        state = XML_PARSER_PROLOG_ATTRIBUTE_VALUE;
        break;
      case XML_PARSER_PROLOG_ATTRIBUTE_VALUE:
        state = XML_PARSER_PROLOG_ATTRIBUTES;
        break;
#endif
        // default: Treat as content
      }
      break;
    // End tag character
    case '/':
      if ( state == XML_PARSER_START_TAG ) {
        state = XML_PARSER_END_TAG;
      } else if ( state == XML_PARSER_START_TAG_NAME  ) {   // removed || state == XML_PARSER_ATTRIBUTES - dyson 2025
        level++;
#ifndef XML_PICO_MODE
        tagChecksum = 0;
#endif
        state = XML_PARSER_END_TAG;
      }
#ifndef XML_PICO_MODE
      else if (
          state != XML_PARSER_ELEMENT_CONTENT &&
          state != XML_PARSER_ATTRIBUTE_VALUE &&
          state != XML_PARSER_PROLOG_ATTRIBUTE_VALUE
      ) {
        errors |= XML_ERROR_MISSPLACED_END_CHAR;
      }
#endif
      // else: Treat as content
      break;
    // All other characters
    default:
      if ( state == XML_PARSER_START_TAG )
        state = XML_PARSER_START_TAG_NAME;
 
      if ( state == XML_PARSER_PROLOG_TAG )
        state = XML_PARSER_PROLOG_TAG_NAME;
      if ( state == XML_PARSER_START_TAG_NAME ) {
        if (
            level == matchLevel &&
            matchCount == position &&
            level < pathSize &&
            position < strlen_P( path[level] ) &&
            charToParse == ( char ) pgm_read_byte( path[level] + position )
        ) {
          matchCount++;
        }
#ifndef XML_PICO_MODE
        tagChecksum ^= ( uint8_t ) charToParse;
#endif
      }   //end trying to match the tag
#ifndef XML_PICO_MODE
      else if ( state == XML_PARSER_PROLOG_TAG_NAME ) {
        if ( position < ( sizeof( XML_PROLOG ) - 1 ) && charToParse != XML_PROLOG[position] ) {
          errors |= XML_ERROR_MALFORMED_PROLOG;
        }
      } else if ( state == XML_PARSER_END_TAG ) {
        checksum ^= ( uint8_t ) charToParse;
      }
#endif
      // else: Treat as content or ignore
      position++;
      break;
    }
  }

#if DEBUG_PARSE_XPATH
        Serial.print( charToParse );
if (state != oldstate){
      Serial.print("                   ");    Serial.print(state);      Serial.print("  "); Serial.print(level); Serial.print(" -  "); Serial.println(matchLevel);
 
  oldstate= state;
}

#endif


  return
#ifndef XML_PICO_MODE
      errors == 0 &&
#endif
      matchLevel == pathSize;
} // end find function

bool MicroXPath_P::elementPathMatch() {
  return level == matchLevel &&
         position == matchCount &&
         level < pathSize &&
         matchCount == strlen_P( path[level] );
}
