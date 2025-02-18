/************************************************************************/
/* Sonos UPnP, an UPnP based read/write remote control library, v2.1.   */
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
/* Written by Thomas Mittet (code@lookout.no) January 2015.             */
/*                                                                      */
/* Modified for use with Wifi.h or WifiNINA.h for wireless Networking   */
/* Added uPnP Network Scan function  Jay Fox 2020                       */
/* https://github.com/javos65/Sonos-ESP32                               */
/*                                                                      */
/* Updated the sonos track info XPath parameters. This corrects bug     */
/* that incorrectly parse music started from Amazon music. Fixing this  */
/* required correcting XPath that only worked because of the bug.  Dyson*/
/************************************************************************/

#include "SonosUPnP.h"

#define DEBUG_XPATH 0 // serial print Data read and write to xPath
//#define DEBUG_XGEN 1       // serial print generic debug info

const char p_HttpVersion[] PROGMEM         = HTTP_VERSION;
const char p_HeaderHost[] PROGMEM          = HEADER_HOST;
const char p_HeaderContentType[] PROGMEM   = HEADER_CONTENT_TYPE;
const char p_HeaderContentLength[] PROGMEM = HEADER_CONTENT_LENGTH;
const char p_HeaderSoapAction[] PROGMEM    = HEADER_SOAP_ACTION;
const char p_HeaderConnection[] PROGMEM    = HEADER_CONNECTION;

const char p_SoapEnvelopeStart[] PROGMEM = SOAP_ENVELOPE_START;
const char p_SoapEnvelopeEnd[] PROGMEM   = SOAP_ENVELOPE_END;
const char p_SoapBodyStart[] PROGMEM     = SOAP_BODY_START;
const char p_SoapBodyEnd[] PROGMEM       = SOAP_BODY_END;
const char p_SoapEnvelope[] PROGMEM      = SOAP_TAG_ENVELOPE;
const char p_SoapBody[] PROGMEM          = SOAP_TAG_BODY;

const char p_UpnpUrnSchema[] PROGMEM                = UPNP_URN_SCHEMA;
const char p_UpnpAvTransportService[] PROGMEM       = UPNP_AV_TRANSPORT_SERVICE;
const char p_UpnpAvTransportEndpoint[] PROGMEM      = UPNP_AV_TRANSPORT_ENDPOINT;
const char p_UpnpRenderingControlService[] PROGMEM  = UPNP_RENDERING_CONTROL_SERVICE;
const char p_UpnpRenderingControlEndpoint[] PROGMEM = UPNP_RENDERING_CONTROL_ENDPOINT;
const char p_UpnpDevicePropertiesService[] PROGMEM  = UPNP_DEVICE_PROPERTIES_SERVICE;
const char p_UpnpDevicePropertiesEndpoint[] PROGMEM = UPNP_DEVICE_PROPERTIES_ENDPOINT;

const char p_GetZoneAttributesA[] = PROGMEM SONOS_TAG_GET_ZONE_ATTR;
const char p_GetZoneAttributesR[] = PROGMEM SONOS_TAG_GET_ZONE_ATTR_RESPONSE;
const char p_ZoneName[]           = PROGMEM SONOS_TAG_ZONENAME;
const char p_GetZoneInfoA[]       = PROGMEM SONOS_TAG_GET_ZONE_INFO;
const char p_GetZoneInfoR[]       = PROGMEM SONOS_TAG_GET_ZONE_INFO_RESPONSE;
const char p_Serial[]             = PROGMEM SONOS_TAG_SERIAL;

const char p_ZPSupportInfo[] = PROGMEM SONOS_GET_ZPSUPPORTINFO;
const char p_ZPInfo[]        = PROGMEM SONOS_GET_ZPINFO;
const char p_ZPZone[]        = PROGMEM SONOS_GET_ZPZONE;
const char p_ZPLocalUID[]    = PROGMEM SONOS_GET_ZPLOCALUID;
const char p_ZPSerial[]      = PROGMEM SONOS_GET_ZPSERIAL;
const char p_ZPSeriesID[]    = PROGMEM SONOS_GET_ZPSERIESID;

const char p_Play[] PROGMEM                               = SONOS_TAG_PLAY;
const char p_SourceRinconTemplate[] PROGMEM               = SONOS_SOURCE_RINCON_TEMPLATE;
const char p_Stop[] PROGMEM                               = SONOS_TAG_STOP;
const char p_Pause[] PROGMEM                              = SONOS_TAG_PAUSE;
const char p_Previous[] PROGMEM                           = SONOS_TAG_PREVIOUS;
const char p_Next[] PROGMEM                               = SONOS_TAG_NEXT;
const char p_InstenceId0Tag[] PROGMEM                     = SONOS_INSTANCE_ID_0_TAG;
const char p_Seek[] PROGMEM                               = SONOS_TAG_SEEK;
const char p_SeekModeTagStart[] PROGMEM                   = SONOS_SEEK_MODE_TAG_START;
const char p_SeekModeTagEnd[] PROGMEM                     = SONOS_SEEK_MODE_TAG_END;
const char p_TimeFormatTemplate[] PROGMEM                 = SONOS_TIME_FORMAT_TEMPLATE;
const char p_SetAVTransportURI[] PROGMEM                  = SONOS_TAG_SET_AV_TRANSPORT_URI;
const char p_UriMetaLightStart[] PROGMEM                  = SONOS_URI_META_LIGHT_START;
const char p_UriMetaLightEnd[] PROGMEM                    = SONOS_URI_META_LIGHT_END;
const char p_RadioMetaFullStart[] PROGMEM                 = SONOS_RADIO_META_FULL_START;
const char p_RadioMetaFullEnd[] PROGMEM                   = SONOS_RADIO_META_FULL_END;
const char p_BecomeCoordinatorOfStandaloneGroup[] PROGMEM = SONOS_TAG_BECOME_COORDINATOR_OF_STANDALONE_GROUP;
const char p_SetLEDState[] PROGMEM                        = SONOS_TAG_SET_LED_STATE;

const char p_AddURIToQueue[] PROGMEM            = SONOS_TAG_ADD_URI_TO_QUEUE;
const char p_SavedQueues[] PROGMEM              = SONOS_SAVED_QUEUES;
const char p_RemoveAllTracksFromQueue[] PROGMEM = SONOS_TAG_REMOVE_ALL_TRACKS_FROM_QUEUE;
const char p_PlaylistMetaLightStart[] PROGMEM   = SONOS_PLAYLIST_META_LIGHT_START;
const char p_PlaylistMetaLightEnd[] PROGMEM     = SONOS_PLAYLIST_META_LIGHT_END;
const char p_PlaylistMetaFullStart[] PROGMEM    = SONOS_PLAYLIST_META_FULL_START;
const char p_PlaylistMetaFullEnd[] PROGMEM      = SONOS_PLAYLIST_META_FULL_END;

const char p_GetPositionInfoA[] PROGMEM = SONOS_TAG_GET_POSITION_INFO;
const char p_GetPositionInfoR[] PROGMEM = SONOS_TAG_GET_POSITION_INFO_RESPONSE;
const char p_Track[] PROGMEM            = SONOS_TAG_TRACK;
const char p_TrackDuration[] PROGMEM    = SONOS_TAG_TRACK_DURATION;
const char p_TrackURI[] PROGMEM         = SONOS_TAG_TRACK_URI;
const char p_RelTime[] PROGMEM          = SONOS_TAG_REL_TIME;
const char p_TrackMeta[] PROGMEM        = SONOS_TAG_TRACKMETA;
const char p_DIDL[] PROGMEM             = SONOS_TAG_DIDL_LITE;
const char p_Item[] PROGMEM             = SONOS_TAG_ITEM;
const char p_TrackTitle[] PROGMEM       = SONOS_ATTRIB_TITLE;
const char p_TrackCreator[] PROGMEM     = SONOS_ATTRIB_CREATOR;
const char p_TrackAlbum[] PROGMEM       = SONOS_ATTRIB_ALBUM;
const char p_TrackArtist[] PROGMEM      = SONOS_ATTRIB_ARTIST;

const char p_GetMuteA[] PROGMEM        = SONOS_TAG_GET_MUTE;
const char p_GetMuteR[] PROGMEM        = SONOS_TAG_GET_MUTE_RESPONSE;
const char p_CurrentMute[] PROGMEM     = SONOS_TAG_CURRENT_MUTE;
const char p_GetVolumeA[] PROGMEM      = SONOS_TAG_GET_VOLUME;
const char p_GetVolumeR[] PROGMEM      = SONOS_TAG_GET_VOLUME_RESPONSE;
const char p_CurrentVolume[] PROGMEM   = SONOS_TAG_CURRENT_VOLUME;
const char p_GetOutputFixedA[] PROGMEM = SONOS_TAG_GET_OUTPUT_FIXED;
const char p_GetOutputFixedR[] PROGMEM = SONOS_TAG_GET_FIXED_RESPONSE;
const char p_CurrentFixed[] PROGMEM    = SONOS_TAG_CURRENT_FIXED;
const char p_GetBassA[] PROGMEM        = SONOS_TAG_GET_BASS;
const char p_GetBassR[] PROGMEM        = SONOS_TAG_GET_BASS_RESPONSE;
const char p_CurrentBass[] PROGMEM     = SONOS_TAG_CURRENT_BASS;
const char p_GetTrebleA[] PROGMEM      = SONOS_TAG_GET_TREBLE;
const char p_GetTrebleR[] PROGMEM      = SONOS_TAG_GET_TREBLE_RESPONSE;
const char p_CurrentTreble[] PROGMEM   = SONOS_TAG_CURRENT_TREBLE;
const char p_GetLoudnessA[] PROGMEM    = SONOS_TAG_GET_LOUDNESS;
const char p_GetLoudnessR[] PROGMEM    = SONOS_TAG_GET_LOUDNESS_RESPONSE;
const char p_CurrentLoudness[] PROGMEM = SONOS_TAG_CURRENT_LOUDNESS;

const char p_SetMute[] PROGMEM         = SONOS_TAG_SET_MUTE;
const char p_SetVolume[] PROGMEM       = SONOS_TAG_SET_VOLUME;
const char p_SetBass[] PROGMEM         = SONOS_TAG_SET_BASS;
const char p_SetTreble[] PROGMEM       = SONOS_TAG_SET_TREBLE;
const char p_SetLoudness[] PROGMEM     = SONOS_TAG_SET_LOUDNESS;
const char p_ChannelTagStart[] PROGMEM = SONOS_CHANNEL_TAG_START;
const char p_ChannelTagEnd[] PROGMEM   = SONOS_CHANNEL_TAG_END;

const char p_GetTransportSettingsA[] PROGMEM = SONOS_TAG_GET_TRANSPORT_SETTINGS;
const char p_GetTransportSettingsR[] PROGMEM = SONOS_TAG_GET_TRANSPORT_SETTINGS_RESPONSE;
const char p_PlayMode[] PROGMEM              = SONOS_TAG_PLAY_MODE;
const char p_SetPlayMode[] PROGMEM           = SONOS_TAG_SET_PLAY_MODE;

const char p_GetTransportInfoA[] PROGMEM     = SONOS_TAG_GET_TRANSPORT_INFO;
const char p_GetTransportInfoR[] PROGMEM     = SONOS_TAG_GET_TRANSPORT_INFO_RESPONSE;
const char p_CurrentTransportState[] PROGMEM = SONOS_TAG_CURRENT_TRANSPORT_STATE;

const char p_getMediaInfoA[] PROGMEM = SONOS_TAG_GET_MEDIA_INFO;
const char p_getMediaInfoR[] PROGMEM = SONOS_TAG_GET_MEDIA_INFO_RESPONSE;
const char p_CurrentMedium[] PROGMEM = SONOS_TAG_MEDIUM_STATUS;
const char p_CurrentArtist[] PROGMEM = SONOS_TAG_ARTIST_STATUS;

const uint8_t p_UPnPBroadcast[] PROGMEM = UPNP_DEVICE_SCAN;

const char *p_MediaSource[SONOS_MAXSOURCE] = { SONOS_SOURCE_UNKNOWN_SCHEME, SONOS_SOURCE_FILE_SCHEME, SONOS_SOURCE_SPOTIFY_SCHEME, SONOS_SOURCE_HTTP_SCHEME, SONOS_SOURCE_RADIO_SCHEME, SONOS_SOURCE_RADIO_AAC_SCHEME, SONOS_SOURCE_LINEIN_SCHEME, SONOS_SOURCE_MASTER_SCHEME, SONOS_SOURCE_QUEUE_SCHEME, SONOS_SOURCE_SPOTIFYSTATION_SCHEME, SONOS_SOURCE_LOCALHTTP_SCHEME };

const char *p_MediaSourceName[SONOS_MAXSOURCE] = { UNKNOWN_SCHEME, FILE_SCHEME, SPOTIFY_SCHEME, HTTP_SCHEME, RADIO_SCHEME, RADIO_AAC_SCHEME, LINEIN_SCHEME, MASTER_SCHEME, QUEUE_SCHEME, SPOTIFYSTATION_SCHEME, LOCALHTTP_SCHEME };

// new local Data Variables - is increasing footprint, but easies passing of extra char-string-info
char CREATOR_BUFFER[76]  = "\0";
char ARTIST_BUFFER[76]   = "\0";
char TITLE_BUFFER[100]   = "\0";
char ALBUM_BUFFER[92]    = "\0";
char DURATION_BUFFER[16] = "\0";
char POSITION_BUFFER[16] = "\0";
char ZONE_BUFFER[32]     = "\0";
char UID_BUFFER[32]      = "\0";
char SERIAL_BUFFER[24]   = "\0";
char SERIESID_BUFFER[24] = "\0";
char MEDIUM_BUFFER[16]   = "\0";
char STATUS_BUFFER[16]   = "\0";
char PLAYMODE_BUFFER[16] = "\0";
char SOURCE_BUFFER[16]   = "\0";

SonosUPnP::SonosUPnP( WiFiClient client, void ( *ethernetErrCallback )( void ) ) // wireless adapted JV //GS added error callback
{
#ifndef SONOS_WRITE_ONLY_MODE
  this->xPath = MicroXPath_P();
#endif
  this->ethClient           = client;
  this->ethernetErrCallback = ethernetErrCallback; // adapted JV
}

void SonosUPnP::setAVTransportURI( IPAddress speakerIP, const char *scheme, const char *address ) {
  setAVTransportURI( speakerIP, scheme, address, p_UriMetaLightStart, p_UriMetaLightEnd, "" );
}

void SonosUPnP::seekTrack( IPAddress speakerIP, uint16_t index ) {
  char indexChar[6];
  itoa( index, indexChar, 10 );
  seek( speakerIP, SONOS_SEEK_MODE_TRACK_NR, indexChar );
}

void SonosUPnP::seekTime( IPAddress speakerIP, uint8_t hour, uint8_t minute, uint8_t second ) {
  char time[11];
  sprintf_P( time, p_TimeFormatTemplate, hour, minute, second );
  seek( speakerIP, SONOS_SEEK_MODE_REL_TIME, time );
}

void SonosUPnP::setPlayMode( IPAddress speakerIP, uint8_t playMode ) {
  const char *playModeValue;
  switch ( playMode ) {
  case SONOS_PLAY_MODE_REPEAT:
    playModeValue = SONOS_PLAY_MODE_REPEAT_VALUE;
    break;
  case SONOS_PLAY_MODE_SHUFFLE_REPEAT:
    playModeValue = SONOS_PLAY_MODE_SHUFFLE_REPEAT_VALUE;
    break;
  case SONOS_PLAY_MODE_SHUFFLE:
    playModeValue = SONOS_PLAY_MODE_SHUFFLE_VALUE;
    break;
  default:
    playModeValue = SONOS_PLAY_MODE_NORMAL_VALUE;
    break;
  }
  upnpSet( speakerIP, UPNP_AV_TRANSPORT, p_SetPlayMode, SONOS_TAG_NEW_PLAY_MODE, playModeValue );
}

void SonosUPnP::play( IPAddress speakerIP ) {
  upnpSet( speakerIP, UPNP_AV_TRANSPORT, p_Play, SONOS_TAG_SPEED, "1" );
}

void SonosUPnP::playFile( IPAddress speakerIP, const char *path ) {
  setAVTransportURI( speakerIP, SONOS_SOURCE_FILE_SCHEME, path );
  play( speakerIP );
}

void SonosUPnP::playHttp( IPAddress speakerIP, const char *address ) {
  setAVTransportURI( speakerIP, SONOS_SOURCE_HTTP_SCHEME, address );
  play( speakerIP );
}

void SonosUPnP::playRadio( IPAddress speakerIP, const char *address, const char *title ) {
  setAVTransportURI( speakerIP, SONOS_SOURCE_RADIO_SCHEME, address, p_RadioMetaFullStart, p_RadioMetaFullEnd, title );
  play( speakerIP );
}

void SonosUPnP::playLineIn( IPAddress speakerIP, const char *speakerID ) {
  char address[30];
  sprintf_P( address, p_SourceRinconTemplate, speakerID, UPNP_PORT, "" );
  setAVTransportURI( speakerIP, SONOS_SOURCE_LINEIN_SCHEME, address );
  play( speakerIP );
}

void SonosUPnP::playQueue( IPAddress speakerIP, const char *speakerID ) {
  char address[30];
  sprintf_P( address, p_SourceRinconTemplate, speakerID, UPNP_PORT, "#0" );
  setAVTransportURI( speakerIP, SONOS_SOURCE_QUEUE_SCHEME, address );
  play( speakerIP );
}

void SonosUPnP::playConnectToMaster( IPAddress speakerIP, const char *masterSpeakerID ) {
  char address[30];
  sprintf_P( address, p_SourceRinconTemplate, masterSpeakerID, UPNP_PORT, "" );
  setAVTransportURI( speakerIP, SONOS_SOURCE_MASTER_SCHEME, address );
}

void SonosUPnP::disconnectFromMaster( IPAddress speakerIP ) {
  upnpSet( speakerIP, UPNP_AV_TRANSPORT, p_BecomeCoordinatorOfStandaloneGroup );
}

void SonosUPnP::stop( IPAddress speakerIP ) {
  upnpSet( speakerIP, UPNP_AV_TRANSPORT, p_Stop );
}

void SonosUPnP::pause( IPAddress speakerIP ) {
  upnpSet( speakerIP, UPNP_AV_TRANSPORT, p_Pause );
}

void SonosUPnP::skip( IPAddress speakerIP, uint8_t direction ) {
  upnpSet(
      speakerIP, UPNP_AV_TRANSPORT, direction == SONOS_DIRECTION_FORWARD ? p_Next : p_Previous
  );
}

void SonosUPnP::setMute( IPAddress speakerIP, bool state ) {
  upnpSet(
      speakerIP, UPNP_RENDERING_CONTROL, p_SetMute,
      SONOS_TAG_DESIRED_MUTE, state ? "1" : "0", "", p_ChannelTagStart, p_ChannelTagEnd, SONOS_CHANNEL_MASTER
  );
}

void SonosUPnP::setVolume( IPAddress speakerIP, uint8_t volume ) {
  setVolume( speakerIP, volume, SONOS_CHANNEL_MASTER );
}

void SonosUPnP::setVolume( IPAddress speakerIP, uint8_t volume, const char *channel ) {
  if ( volume > 100 )
    volume = 100;
  char volumeChar[4];
  itoa( volume, volumeChar, 10 );
  upnpSet(
      speakerIP, UPNP_RENDERING_CONTROL, p_SetVolume,
      SONOS_TAG_DESIRED_VOLUME, volumeChar, "", p_ChannelTagStart, p_ChannelTagEnd, channel
  );
}

void SonosUPnP::setBass( IPAddress speakerIP, int8_t bass ) {
  bass = constrain( bass, -10, 10 );
  char bassChar[4];
  itoa( bass, bassChar, 10 );
  upnpSet(
      speakerIP, UPNP_RENDERING_CONTROL, p_SetBass,
      SONOS_TAG_DESIRED_BASS, bassChar
  );
}

void SonosUPnP::setTreble( IPAddress speakerIP, int8_t treble ) {
  treble = constrain( treble, -10, 10 );
  char trebleChar[4];
  itoa( treble, trebleChar, 10 );
  upnpSet(
      speakerIP, UPNP_RENDERING_CONTROL, p_SetTreble,
      SONOS_TAG_DESIRED_TREBLE, trebleChar
  );
}

void SonosUPnP::setLoudness( IPAddress speakerIP, bool state ) {
  upnpSet(
      speakerIP, UPNP_RENDERING_CONTROL, p_SetLoudness,
      SONOS_TAG_DESIRED_LOUDNESS, state ? "1" : "0", "", p_ChannelTagStart, p_ChannelTagEnd, SONOS_CHANNEL_MASTER
  );
}

void SonosUPnP::setStatusLight( IPAddress speakerIP, bool state ) {
  upnpSet(
      speakerIP, UPNP_DEVICE_PROPERTIES, p_SetLEDState,
      SONOS_TAG_DESIRED_LED_STATE, state ? "On" : "Off"
  );
}

void SonosUPnP::addPlaylistToQueue( IPAddress speakerIP, uint16_t playlistIndex ) {
  char path[45];
  sprintf_P( path, p_SavedQueues, playlistIndex );
  addTrackToQueue( speakerIP, "", path );
}

void SonosUPnP::addTrackToQueue( IPAddress speakerIP, const char *scheme, const char *address ) {
  upnpSet(
      speakerIP, UPNP_AV_TRANSPORT, p_AddURIToQueue,
      SONOS_TAG_ENQUEUED_URI, scheme, address, p_PlaylistMetaLightStart, p_PlaylistMetaLightEnd, ""
  );
}

void SonosUPnP::removeAllTracksFromQueue( IPAddress speakerIP ) {
  upnpSet( speakerIP, UPNP_AV_TRANSPORT, p_RemoveAllTracksFromQueue );
}

#ifndef SONOS_WRITE_ONLY_MODE

void SonosUPnP::setRepeat( IPAddress speakerIP, bool repeat ) {
  bool current = getRepeat( speakerIP );
  if ( repeat != current ) {
    setPlayMode( speakerIP, current ^ SONOS_PLAY_MODE_REPEAT );
  }
}

void SonosUPnP::setShuffle( IPAddress speakerIP, bool shuffle ) {
  bool current = getShuffle( speakerIP );
  if ( shuffle != current ) {
    setPlayMode( speakerIP, current ^ SONOS_PLAY_MODE_SHUFFLE );
  }
}

void SonosUPnP::toggleRepeat( IPAddress speakerIP ) {
  setPlayMode( speakerIP, getPlayMode( speakerIP ) ^ SONOS_PLAY_MODE_REPEAT );
}

void SonosUPnP::toggleShuffle( IPAddress speakerIP ) {
  setPlayMode( speakerIP, getPlayMode( speakerIP ) ^ SONOS_PLAY_MODE_SHUFFLE );
}

void SonosUPnP::togglePause( IPAddress speakerIP ) {
  uint8_t state = getState( speakerIP );
  if ( state == SONOS_STATE_PLAYING ) {
    pause( speakerIP );
  } else if ( state == SONOS_STATE_PAUSED ) {
    play( speakerIP );
  }
}

void SonosUPnP::toggleMute( IPAddress speakerIP ) {
  setMute( speakerIP, !getMute( speakerIP ) );
}

void SonosUPnP::toggleLoudness( IPAddress speakerIP ) {
  setLoudness( speakerIP, !getLoudness( speakerIP ) );
}

// JV
// New function : fill Sonos info  Structure with info
// 1. use parse HTTP:/[ip.nu.mb.er]:1400/status/zp command -
// 2.
SonosInfo SonosUPnP::getSonosInfo( IPAddress speakerIP ) {
  SonosInfo ZP;
  ZONE_BUFFER[0]     = 0;
  UID_BUFFER[0]      = 0;
  SERIAL_BUFFER[0]   = 0;
  SERIESID_BUFFER[0] = 0; // set all buffers to string "0"
  if ( upnpGetzp( speakerIP ) ) {
    xPath.reset();
    char infoBuffer[20] = "";
    // Zone Info
    ZP.zone       = ZONE_BUFFER;
    PGM_P zpath[] = { p_ZPSupportInfo, p_ZPInfo, p_ZPZone };
    ethClient_xPath( zpath, 3, ZONE_BUFFER, sizeof( ZONE_BUFFER ) );
    // Local UIDInfo
    ZP.uid        = UID_BUFFER;
    PGM_P ypath[] = { p_ZPSupportInfo, p_ZPInfo, p_ZPLocalUID };
    ethClient_xPath( ypath, 3, UID_BUFFER, sizeof( UID_BUFFER ) );
    // Serial Info
    ZP.serial     = SERIAL_BUFFER;
    PGM_P xpath[] = { p_ZPSupportInfo, p_ZPInfo, p_ZPSerial };
    ethClient_xPath( xpath, 3, SERIAL_BUFFER, sizeof( SERIAL_BUFFER ) );
    // Series Info
    ZP.seriesid   = SERIESID_BUFFER;
    PGM_P wpath[] = { p_ZPSupportInfo, p_ZPInfo, p_ZPSeriesID };
    ethClient_xPath( wpath, 3, SERIESID_BUFFER, sizeof( SERIESID_BUFFER ) );
  }
  ethClient_stop();
  ZP.status = STATUS_BUFFER;
  getState( speakerIP, STATUS_BUFFER );
  ZP.medium = MEDIUM_BUFFER;
  getMedium( speakerIP, MEDIUM_BUFFER );
  ZP.source = SOURCE_BUFFER;
  getSource( speakerIP, SOURCE_BUFFER );
  ZP.playmode = PLAYMODE_BUFFER;
  getPlayMode( speakerIP, PLAYMODE_BUFFER );
  return ZP;
}

uint8_t SonosUPnP::getState( IPAddress speakerIP ) // Original
{
  PGM_P path[]    = { p_SoapEnvelope, p_SoapBody, p_GetTransportInfoR, p_CurrentTransportState };
  char result[16] = "\0";
  upnpGetString( speakerIP, UPNP_AV_TRANSPORT, p_GetTransportInfoA, "", "", path, 4, result, sizeof( result ) );
  return convertState( result );
}

uint8_t SonosUPnP::getState( IPAddress speakerIP, char *buf ) // New JV : string passthrough
{
  int t;
  PGM_P path[]    = { p_SoapEnvelope, p_SoapBody, p_GetTransportInfoR, p_CurrentTransportState };
  char result[16] = "\0";
  upnpGetString( speakerIP, UPNP_AV_TRANSPORT, p_GetTransportInfoA, "", "", path, 4, result, sizeof( result ) );
  if ( result[0] != 0 ) {
    for ( t = 0; result[t] != 0; ++t )
      buf[t] = result[t];
    buf[t] = 0;
    return convertState( result );
  }
  buf[0] = 0;
  return 0;
}

uint8_t SonosUPnP::getMedium( IPAddress speakerIP ) // New JV Medium state
{
  PGM_P path[]    = { p_SoapEnvelope, p_SoapBody, p_getMediaInfoR, p_CurrentMedium };
  char result[16] = "\0";
  upnpGetString( speakerIP, UPNP_AV_TRANSPORT, p_getMediaInfoA, "", "", path, 4, result, sizeof( result ) );
  return convertMedium( result );
}

uint8_t SonosUPnP::getMedium( IPAddress speakerIP, char *buf ) // New JV Medium with string passthrough
{
  int t;
  PGM_P path[]    = { p_SoapEnvelope, p_SoapBody, p_getMediaInfoR, p_CurrentMedium };
  char result[16] = "\0";
  upnpGetString( speakerIP, UPNP_AV_TRANSPORT, p_getMediaInfoA, "", "", path, 4, result, sizeof( result ) );
  if ( result[0] != 0 ) {
    for ( t = 0; result[t] != 0; ++t )
      buf[t] = result[t];
    buf[t] = 0;
    return convertMedium( result );
  }
  buf[0] = 0;
  return 0;
}

bool SonosUPnP::getZone( IPAddress speakerIP, char *buf ) // New JV : string passthrough
{
  int t;
  PGM_P path[]    = { p_SoapEnvelope, p_SoapBody, p_GetZoneAttributesR, p_ZoneName };
  char result[32] = "\0";
  upnpGetString( speakerIP, UPNP_DEVICE_PROPERTIES, p_GetZoneAttributesA, "", "", path, 4, result, sizeof( result ) );
  if ( result[0] != 0 ) {
    for ( t = 0; result[t] != 0; ++t )
      buf[t] = result[t];
    buf[t] = 0;
    return 1;
  }
  buf[0] = 0;
  return 0;
}

bool SonosUPnP::getSerial( IPAddress speakerIP, char *buf ) // New JV : string passthrough
{
  int t;
  PGM_P path[]    = { p_SoapEnvelope, p_SoapBody, p_GetZoneInfoR, p_Serial };
  char result[20] = "\0";
  upnpGetString( speakerIP, UPNP_DEVICE_PROPERTIES, p_GetZoneInfoA, "", "", path, 4, result, sizeof( result ) );
  if ( result[0] != 0 ) {
    for ( t = 0; result[t] != 0; ++t )
      buf[t] = result[t];
    buf[t] = 0;
    return 1;
  }
  buf[0] = 0;
  return 0;
}

uint8_t SonosUPnP::getPlayMode( IPAddress speakerIP, char *buf ) // New JV : string passthrough
{
  int t;
  PGM_P path[]    = { p_SoapEnvelope, p_SoapBody, p_GetTransportSettingsR, p_PlayMode };
  char result[16] = "\0";
  upnpGetString( speakerIP, UPNP_AV_TRANSPORT, p_GetTransportSettingsA, "", "", path, 4, result, sizeof( result ) );
  if ( result[0] != 0 ) {
    for ( t = 0; result[t] != 0; ++t )
      buf[t] = result[t];
    buf[t] = 0;
    return convertPlayMode( result );
  }
  buf[0] = 0;
  return 0;
}

uint8_t SonosUPnP::getPlayMode( IPAddress speakerIP ) // original
{
  PGM_P path[]    = { p_SoapEnvelope, p_SoapBody, p_GetTransportSettingsR, p_PlayMode };
  char result[16] = "";
  upnpGetString( speakerIP, UPNP_AV_TRANSPORT, p_GetTransportSettingsA, "", "", path, 4, result, sizeof( result ) );
  return convertPlayMode( result );
}

bool SonosUPnP::getRepeat( IPAddress speakerIP ) {
  return getPlayMode( speakerIP ) & SONOS_PLAY_MODE_REPEAT;
}

bool SonosUPnP::getShuffle( IPAddress speakerIP ) {
  return getPlayMode( speakerIP ) & SONOS_PLAY_MODE_SHUFFLE;
}

// provides track number, URI, duration and position (time) in track
TrackInfo SonosUPnP::getTrackInfo( IPAddress speakerIP, char *uriBuffer, size_t uriBufferSize ) {
  TrackInfo trackInfo;
  if ( upnpPost( speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", "", 0, 0, "" ) ) {
    xPath.reset();
    char infoBuffer[20] = "";
    // Track number
    PGM_P npath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_Track };
    ethClient_xPath( npath, 4, infoBuffer, sizeof( infoBuffer ) );
    trackInfo.number = atoi( infoBuffer );
    // Track duration
    PGM_P dpath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackDuration };
    ethClient_xPath( dpath, 4, infoBuffer, sizeof( infoBuffer ) );

    trackInfo.duration = getTimeInSeconds( infoBuffer );
    // Track URI
    PGM_P upath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackURI };
    ethClient_xPath( upath, 4, uriBuffer, uriBufferSize );
    trackInfo.uri = uriBuffer;
    // Track position
    PGM_P ppath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_RelTime };
    ethClient_xPath( ppath, 4, infoBuffer, sizeof( infoBuffer ) );
    trackInfo.position = getTimeInSeconds( infoBuffer );
  }
  ethClient_stop();
  return trackInfo;
}

// JV
// New function to pass full track info including Artist, Album and Songname. Uses modified Xpath parsing
// PArsed from SERVICE : AVTRAnsport -> GetPositionInfo -> XLM response incl TrackMetaData
// this takes two calls because of the way the xml processors works

FullTrackInfo SonosUPnP::getFullTrackInfo( IPAddress speakerIP ) {
  FullTrackInfo trackInfo;
  
  if ( upnpPost( speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", "", 0, 0, "" ) ) {
    xPath.reset();
    // char infoBuffer[20] = "";
    char infoBuffer[100] = ""; // gs increased buffer
    // Track number
    PGM_P npath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_Track };
    ethClient_xPath( npath, 4, infoBuffer, sizeof( infoBuffer ) );
    trackInfo.number = atoi( infoBuffer );
    // Track duration

    trackInfo.duration = DURATION_BUFFER;

    PGM_P dpath[]      = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackDuration };
    ethClient_xPath( dpath, 4, DURATION_BUFFER, sizeof( DURATION_BUFFER ) );
    if (!isValidTimeFormat(DURATION_BUFFER)){  //added check on format to handle non player devices.
      strncpy(DURATION_BUFFER,"  ",sizeof(DURATION_BUFFER));
      trackInfo.durationSeconds = 1;
    } else {
      trackInfo.durationSeconds = getTimeInSeconds(DURATION_BUFFER);
    }

    // Track position
    trackInfo.position = POSITION_BUFFER;
    PGM_P ppath[]      = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_RelTime };
    ethClient_xPath( ppath, 4, POSITION_BUFFER, sizeof( POSITION_BUFFER ) );
    if (!isValidTimeFormat(POSITION_BUFFER)){  //added check on format to handle non player devices.
      strncpy(POSITION_BUFFER,"  ",sizeof(POSITION_BUFFER));
      trackInfo.positionSeconds = 0;
    } else {
      trackInfo.positionSeconds = getTimeInSeconds(POSITION_BUFFER);
    }

  
  }
  ethClient_stop();
  
  if ( upnpPost( speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", "", 0, 0, "" ) ) {
    xPath.reset();
    // char infoBuffer[20] = "";
    // Track title
    TITLE_BUFFER[0] = 0;
    PGM_P tpath[]   = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackMeta, p_DIDL, p_Item, p_TrackTitle };
    ethClient_xPath2( tpath, 7, TITLE_BUFFER, sizeof( TITLE_BUFFER ) );
    trackInfo.title = TITLE_BUFFER;
    // Track creator
    CREATOR_BUFFER[0] = 0;
    PGM_P cpath[]     = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackMeta, p_DIDL, p_Item, p_TrackCreator };
    ethClient_xPath2( cpath, 7, CREATOR_BUFFER, sizeof( CREATOR_BUFFER ) );
    trackInfo.creator = CREATOR_BUFFER;
    // Track album
    ALBUM_BUFFER[0] = 0;
    PGM_P apath[]   = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackMeta, p_DIDL, p_Item, p_TrackAlbum };
    ethClient_xPath2( apath, 7, ALBUM_BUFFER, sizeof( ALBUM_BUFFER ) );
    trackInfo.album = ALBUM_BUFFER;
    // Track artist
    ARTIST_BUFFER[0] = 0;
    PGM_P aapath[]   = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackMeta, p_TrackArtist };
    ethClient_xPath2( aapath, 5, ARTIST_BUFFER, sizeof( ARTIST_BUFFER ) );
    if ( ARTIST_BUFFER[0] != 0 )
      trackInfo.creator = ARTIST_BUFFER; // otherwize keep creator attribute
  }
  ethClient_stop();

  return trackInfo;
}

uint16_t SonosUPnP::getTrackNumber( IPAddress speakerIP ) {
  PGM_P path[]   = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_Track };
  char result[6] = "0";
  upnpGetString( speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", path, 4, result, sizeof( result ) );
  return atoi( result );
}

void SonosUPnP::getTrackURI( IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize ) {
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackURI };
  upnpGetString( speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", path, 4, resultBuffer, resultBufferSize );
}

// JV New
void SonosUPnP::getTrackCreator( IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize ) {
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackMeta, p_DIDL, p_Item, p_TrackCreator };
  if ( upnpPost( speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", "", 0, 0, "" ) ) {
    xPath.reset();
    ethClient_xPath2( path, 7, resultBuffer, resultBufferSize );
  }
  ethClient_stop();
}

// JV New 
void SonosUPnP::getTrackTitle( IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize ) {
  PGM_P path[]            = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackMeta, p_DIDL, p_Item, p_TrackTitle };
  if ( upnpPost( speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", "", 0, 0, "" ) ) {
    xPath.reset();
    ethClient_xPath2( path, 7, resultBuffer, resultBufferSize );
    Serial.print( " result of getTrackTitle is " );
    Serial.println( resultBuffer );
  }
  ethClient_stop();
}

// JV New 
void SonosUPnP::getTrackAlbum( IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize ) {
  PGM_P path[]            = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackMeta, p_DIDL, p_Item, p_TrackAlbum };
  if ( upnpPost( speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", "", 0, 0, "" ) ) {
    xPath.reset();
    ethClient_xPath2( path, 7, resultBuffer, resultBufferSize );
    Serial.print( " result of getTrackAlbum is " );
    Serial.println( resultBuffer );
  }
  ethClient_stop();
}

uint8_t SonosUPnP::getSourceFromURI( const char *uri ) // adapted JV
{
  uint8_t t, v;
  for ( t = 0; t < SONOS_MAXSOURCE; ++t ) {
    v = 0;
    while ( p_MediaSource[t][v] != 0 ) {
      if ( p_MediaSource[t][v] != uri[v] )
        break;
      v++;
    }
    if ( p_MediaSource[t][v] == 0 )
      return t; // we have a match
  }
  return ( 0 );
}

uint8_t SonosUPnP::getSource( IPAddress speakerIP ) // adapted JV
{
  uint8_t t, v;
  char uri[32] = "";
  getTrackURI( speakerIP, uri, sizeof( uri ) );
  for ( t = 0; t < SONOS_MAXSOURCE; ++t ) {
    v = 0;
    while ( p_MediaSource[t][v] != 0 ) {
      if ( p_MediaSource[t][v] != uri[v] )
        break;
      v++;
    }
    if ( p_MediaSource[t][v] == 0 )
      return t; // we have a match
  }
  return ( 0 );
}

uint8_t SonosUPnP::getSource( IPAddress speakerIP, char *buf ) // new JV
{
  uint8_t t, u, v;
  char uri[32] = "";
  getTrackURI( speakerIP, uri, sizeof( uri ) );
  for ( t = 0; t < SONOS_MAXSOURCE; ++t ) {
    v = 0;
    while ( p_MediaSource[t][v] != 0 ) {
      if ( p_MediaSource[t][v] != uri[v] )
        break;
      v++;
    }
    if ( p_MediaSource[t][v] == 0 ) // we have a match
    {
      for ( u = 0; p_MediaSourceName[t][u] != 0; u++ )
        buf[u] = p_MediaSourceName[t][u]; // copy name to buffer
      buf[u] = 0;                         // end buffer with zero
      return t;
    }
  }
  buf[0] = 0; // make empty buffer string
  return ( 0 );
}

uint32_t SonosUPnP::getTrackDurationInSeconds( IPAddress speakerIP ) {
  PGM_P path[]    = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackDuration };
  char result[20] = "";
  upnpGetString( speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", path, 4, result, sizeof( result ) );
  return getTimeInSeconds( result );
}

uint32_t SonosUPnP::getTrackPositionInSeconds( IPAddress speakerIP ) {
  PGM_P path[]    = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_RelTime };
  char result[20] = "";
  upnpGetString( speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", path, 4, result, sizeof( result ) );
  return getTimeInSeconds( result );
}

uint16_t SonosUPnP::getTrackPositionPerMille( IPAddress speakerIP ) {
  uint16_t perMille = 0;
  if ( upnpPost( speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", "", 0, 0, "" ) ) {
    char result[20];
    xPath.reset();
    PGM_P dpath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackDuration };
    ethClient_xPath( dpath, 4, result, sizeof( result ) );
    uint32_t duration = getTimeInSeconds( result );
    PGM_P ppath[]     = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_RelTime };
    ethClient_xPath( ppath, 4, result, sizeof( result ) );
    uint32_t position = getTimeInSeconds( result );
    if ( duration && position ) {
      perMille = ( position * 1000 ) / duration;
    }
  }
  ethClient_stop();
  return perMille;
}

bool SonosUPnP::getMute( IPAddress speakerIP ) {
  PGM_P path[]   = { p_SoapEnvelope, p_SoapBody, p_GetMuteR, p_CurrentMute };
  char result[3] = "0";
  upnpGetString(
      speakerIP, UPNP_RENDERING_CONTROL, p_GetMuteA,
      SONOS_TAG_CHANNEL, SONOS_CHANNEL_MASTER, path, 4, result, sizeof( result )
  );
  return strcmp( result, "1" ) == 0;
}

uint8_t SonosUPnP::getVolume( IPAddress speakerIP ) {
  return getVolume( speakerIP, SONOS_CHANNEL_MASTER );
}

uint8_t SonosUPnP::getVolume( IPAddress speakerIP, const char *channel ) {
  PGM_P path[]   = { p_SoapEnvelope, p_SoapBody, p_GetVolumeR, p_CurrentVolume };
  char result[5] = "0";
  upnpGetString(
      speakerIP, UPNP_RENDERING_CONTROL, p_GetVolumeA,
      SONOS_TAG_CHANNEL, channel, path, 4, result, sizeof( result )
  );
  return constrain( atoi( result ), 0, 100 );
}

bool SonosUPnP::getOutputFixed( IPAddress speakerIP ) {
  PGM_P path[]   = { p_SoapEnvelope, p_SoapBody, p_GetOutputFixedR, p_CurrentFixed };
  char result[3] = "0";
  upnpGetString( speakerIP, UPNP_RENDERING_CONTROL, p_GetOutputFixedA, "", "", path, 4, result, sizeof( result ) );
  return strcmp( result, "1" ) == 0;
}

int8_t SonosUPnP::getBass( IPAddress speakerIP ) {
  PGM_P path[]   = { p_SoapEnvelope, p_SoapBody, p_GetBassR, p_CurrentBass };
  char result[5] = "0";
  upnpGetString(
      speakerIP, UPNP_RENDERING_CONTROL, p_GetBassA,
      SONOS_TAG_CHANNEL, SONOS_CHANNEL_MASTER, path, 4, result, sizeof( result )
  );
  return constrain( atoi( result ), -10, 10 );
}

int8_t SonosUPnP::getTreble( IPAddress speakerIP ) {
  PGM_P path[]   = { p_SoapEnvelope, p_SoapBody, p_GetTrebleR, p_CurrentTreble };
  char result[5] = "0";
  upnpGetString(
      speakerIP, UPNP_RENDERING_CONTROL, p_GetTrebleA,
      SONOS_TAG_CHANNEL, SONOS_CHANNEL_MASTER, path, 4, result, sizeof( result )
  );
  return constrain( atoi( result ), -10, 10 );
}

bool SonosUPnP::getLoudness( IPAddress speakerIP ) {
  PGM_P path[]   = { p_SoapEnvelope, p_SoapBody, p_GetLoudnessR, p_CurrentLoudness };
  char result[3] = "0";
  upnpGetString(
      speakerIP, UPNP_RENDERING_CONTROL, p_GetLoudnessA,
      SONOS_TAG_CHANNEL, SONOS_CHANNEL_MASTER, path, 4, result, sizeof( result )
  );
  return strcmp( result, "1" ) == 0;
}

#endif

void SonosUPnP::seek( IPAddress speakerIP, const char *mode, const char *data ) {
  upnpSet(
      speakerIP, UPNP_AV_TRANSPORT, p_Seek,
      SONOS_TAG_TARGET, data, "", p_SeekModeTagStart, p_SeekModeTagEnd, mode
  );
}

void SonosUPnP::setAVTransportURI( IPAddress speakerIP, const char *scheme, const char *address, PGM_P metaStart_P, PGM_P metaEnd_P, const char *metaValue ) {
  // Info to show in player, in DIDL format, can be added as META data
  upnpSet(
      speakerIP, UPNP_AV_TRANSPORT, p_SetAVTransportURI,
      SONOS_TAG_CURRENT_URI, scheme, address, metaStart_P, metaEnd_P, metaValue
  );
}

void SonosUPnP::upnpSet( IPAddress ip, uint8_t upnpMessageType, PGM_P action_P ) {
  upnpSet( ip, upnpMessageType, action_P, "", "" );
}

void SonosUPnP::upnpSet( IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *value ) {
  upnpSet( ip, upnpMessageType, action_P, field, value, "", 0, 0, "" );
}

void SonosUPnP::upnpSet( IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *valueA, const char *valueB, PGM_P extraStart_P, PGM_P extraEnd_P, const char *extraValue ) {
  upnpPost( ip, upnpMessageType, action_P, field, valueA, valueB, extraStart_P, extraEnd_P, extraValue );
  ethClient_stop();
}

bool SonosUPnP::upnpGetzp( IPAddress ip ) // JV new - simple GET status/zp command
{
  if ( !ethClient.connect( ip, UPNP_PORT ) )
    return false;
  char buffer[50];
  ethClient_write( "GET /status/zp HTTP/1.1\n" );
  sprintf_P( buffer, p_HeaderHost, ip[0], ip[1], ip[2], ip[3], UPNP_PORT ); // 29 bytes max
  ethClient_write( buffer );
  ethClient_write( "Connection: close\n" );
  ethClient_write( "\n" );

  uint32_t start = millis();
  while ( !ethClient.available() ) {
    if ( millis() > ( start + UPNP_RESPONSE_TIMEOUT_MS ) ) {
      // if (ethernetErrCallback) ethernetErrCallback();
      return false;
    }
  }
  return true;
}

bool SonosUPnP::upnpPost( IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *valueA, const char *valueB, PGM_P extraStart_P, PGM_P extraEnd_P, const char *extraValue ) {
  if ( !ethClient.connect( ip, UPNP_PORT ) ) { // GS added debug print
    Serial.println( "*SONOS: Unable to connect to the speaker :(" );
    return false;
  }

  // Get UPnP service name
  PGM_P upnpService = getUpnpService( upnpMessageType );

  // Get HTTP content/body length
  uint16_t contentLength =
      sizeof( SOAP_ENVELOPE_START ) - 1 +
      sizeof( SOAP_BODY_START ) - 1 +
      SOAP_ACTION_TAG_LEN +
      ( strlen_P( action_P ) * 2 ) +
      sizeof( UPNP_URN_SCHEMA ) - 1 +
      strlen_P( upnpService ) +
      sizeof( SONOS_INSTANCE_ID_0_TAG ) - 1 +
      sizeof( SOAP_BODY_END ) - 1 +
      sizeof( SOAP_ENVELOPE_END ) - 1;

  // Get length of field
  uint8_t fieldLength = strlen( field );
  if ( fieldLength ) {
    contentLength +=
        SOAP_TAG_LEN +
        ( fieldLength * 2 ) +
        strlen( valueA ) +
        strlen( valueB );
  }

  // Get length of extra field data (e.g. meta data fields)
  if ( extraStart_P ) {
    contentLength +=
        strlen_P( extraStart_P ) +
        strlen( extraValue ) +
        strlen_P( extraEnd_P );
  }
  char buffer[3000]; // GS made buffer bigger, this fixed use of strncpy instead of strlcpy
                     // char buffer[50];

  // Write HTTP start
  ethClient_write( "POST " );
  ethClient_write_P( getUpnpEndpoint( upnpMessageType ), buffer, sizeof( buffer ) );
  ethClient_write_P( p_HttpVersion, buffer, sizeof( buffer ) );

  // Write HTTP header
  sprintf_P( buffer, p_HeaderHost, ip[0], ip[1], ip[2], ip[3], UPNP_PORT ); // 29 bytes max
  ethClient_write( buffer );
  ethClient_write_P( p_HeaderContentType, buffer, sizeof( buffer ) );
  sprintf_P( buffer, p_HeaderContentLength, contentLength ); // 23 bytes max
  ethClient_write( buffer );
  ethClient_write_P( p_HeaderSoapAction, buffer, sizeof( buffer ) );
  ethClient_write_P( p_UpnpUrnSchema, buffer, sizeof( buffer ) );
  ethClient_write_P( upnpService, buffer, sizeof( buffer ) );
  ethClient_write( "#" );
  ethClient_write_P( action_P, buffer, sizeof( buffer ) );
  ethClient_write( HEADER_SOAP_ACTION_END );
  ethClient_write_P( p_HeaderConnection, buffer, sizeof( buffer ) );
  ethClient_write( "\n" );

  // Write HTTP body
  ethClient_write_P( p_SoapEnvelopeStart, buffer, sizeof( buffer ) );
  ethClient_write_P( p_SoapBodyStart, buffer, sizeof( buffer ) );
  ethClient_write( SOAP_ACTION_START_TAG_START );
  ethClient_write_P( action_P, buffer, sizeof( buffer ) );
  ethClient_write( SOAP_ACTION_START_TAG_NS );
  ethClient_write_P( p_UpnpUrnSchema, buffer, sizeof( buffer ) );
  ethClient_write_P( upnpService, buffer, sizeof( buffer ) );
  ethClient_write( SOAP_ACTION_START_TAG_END );
  ethClient_write_P( p_InstenceId0Tag, buffer, sizeof( buffer ) );
  if ( fieldLength ) {
    sprintf( buffer, SOAP_TAG_START, field ); // 18 bytes
    ethClient_write( buffer );
    ethClient_write( valueA );
    ethClient_write( valueB );
    sprintf( buffer, SOAP_TAG_END, field ); // 19 bytes
    ethClient_write( buffer );
  }
  if ( extraStart_P ) {
    ethClient_write_P( extraStart_P, buffer, sizeof( buffer ) ); // 390 bytes
    ethClient_write( extraValue );
    ethClient_write_P( extraEnd_P, buffer, sizeof( buffer ) ); // 271 bytes
  }
  ethClient_write( SOAP_ACTION_END_TAG_START );
  ethClient_write_P( action_P, buffer, sizeof( buffer ) ); // 35 bytes
  ethClient_write( SOAP_ACTION_END_TAG_END );
  ethClient_write_P( p_SoapBodyEnd, buffer, sizeof( buffer ) );     // 10 bytes
  ethClient_write_P( p_SoapEnvelopeEnd, buffer, sizeof( buffer ) ); // 14 bytes

  uint32_t start = millis();
  while ( !ethClient.available() ) {
    if ( millis() > ( start + UPNP_RESPONSE_TIMEOUT_MS ) ) {
      // if (ethernetErrCallback) ethernetErrCallback();
      return false;
    }
  }
  return true;
}

PGM_P SonosUPnP::getUpnpService( uint8_t upnpMessageType ) {
  switch ( upnpMessageType ) {
  case UPNP_AV_TRANSPORT:
    return p_UpnpAvTransportService;
  case UPNP_RENDERING_CONTROL:
    return p_UpnpRenderingControlService;
  case UPNP_DEVICE_PROPERTIES:
    return p_UpnpDevicePropertiesService;
  }
}

PGM_P SonosUPnP::getUpnpEndpoint( uint8_t upnpMessageType ) {
  switch ( upnpMessageType ) {
  case UPNP_AV_TRANSPORT:
    return p_UpnpAvTransportEndpoint;
  case UPNP_RENDERING_CONTROL:
    return p_UpnpRenderingControlEndpoint;
  case UPNP_DEVICE_PROPERTIES:
    return p_UpnpDevicePropertiesEndpoint;
  }
}

void SonosUPnP::ethClient_write( const char *data ) {
  ethClient.print( data );
}

void SonosUPnP::ethClient_write_P( PGM_P data_P, char *buffer, size_t bufferSize ) {

  uint16_t dataLen = strlen_P( data_P );
  uint16_t dataPos = 0;
  while ( dataLen > dataPos ) {
    // strlcpy_P(buffer, data_P + dataPos, bufferSize);
    strncpy_P( buffer, data_P + dataPos, bufferSize ); // GS using joeybab3 fork to replace strlcpy
    ethClient.print( buffer );

    dataPos += bufferSize - 1;
  }
}

void SonosUPnP::ethClient_stop() {
  if ( ethClient ) {
     char c;
    while ( ethClient.available() )
        c = ethClient.read(); // Read each byte
       // Serial.print(c);           // (Optional) Print it to debug/log
    ethClient.stop();
  }
}

// JV : SSDP over UDP to scan for uPnP on port 1400 : Sonos, and load then into the IP list
uint8_t SonosUPnP::CheckUPnP( IPAddress *List, int Listsize ) {
#define SSDPUDP_TIMEOUT 15 // TD changed to get all sonos units.  orignal was 8
  int u, n, t                         = 0;
  uint8_t match, buffercounter, found = 0;
  char c;
  char udpbuffer[33];
  udpbuffer[32] = 0; // mark last buffer item as a zero
  WiFiUDP SSDP_UDP;  // A UDP instance to let us send and receive packets over UDP
  IPAddress tmpIP;
  // broadcast  a discovery packet to get the Sonos Zone Players to respond
  Serial.print("start scan");
  SSDP_UDP.begin( 1900 );
  SSDP_UDP.beginPacket( IPAddress( 239, 255, 255, 250 ), 1900 ); // SSDP request at port 1900
  SSDP_UDP.write( p_UPnPBroadcast, sizeof( p_UPnPBroadcast ) );
  SSDP_UDP.endPacket();
#if DEBUG_XGEN
  Serial.println( "* SSDP : UDP packet send 239.255.255.250:1900" );
#endif
  while ( t < SSDPUDP_TIMEOUT ) { // wait to see if a reply is available
    t++;
    vTaskDelay(50 / portTICK_PERIOD_MS);; // gs increased, 
    while ( n = SSDP_UDP.parsePacket() ) {
#if DEBUG_XPATH
      Serial.print( "* Reading packet of " );
      Serial.print( n );
      Serial.println( "bytes" );
#endif
      char string1[] = ":1400";
      match          = 0;
      buffercounter  = 0;
      for ( u = 0; u < n; ++u ) {
        SSDP_UDP.read( &c, 1 );                                // Serial.print(c);     // read the packet into the buffer
        buffercounter            = ( buffercounter + 1 ) % 32; // store data in looping buffer of 32
        udpbuffer[buffercounter] = c;
        if ( c == string1[match] )
          match++;
        else
          match = 0;
        if ( match == sizeof( string1 ) - 1 ) /// ! found one :1400
        {
#if DEBUG_XGEN
          Serial.print( "* uPnP:1400 found " );
#endif
          readback_IP( &tmpIP, udpbuffer, buffercounter, 32 );
          // Check for duplicate IPs before adding
          bool isDuplicate = false;
          for ( int i = 0; i < found; i++ ) {
            if ( List[i] == tmpIP ) {
              isDuplicate = true;
              break;
            }
          }
          if ( !isDuplicate ) {
            if ( found < Listsize ) {
              List[found] = tmpIP;
              found++;
#if DEBUG_XGEN
              Serial.print( "* IPAddress:" );
              Serial.println( tmpIP );
#endif
            } else {
#if DEBUG_XGEN
              Serial.println( "* too many uPnP found " );
#endif
            }
          }  // is not duplicate
        } // if found :1400
      } // for-loop packet data
    } // while packet data is available
    if ( t > SSDPUDP_TIMEOUT )
      break; // max <UDP_TIMEOUT> times to parse packed, otherwise skip
  }
  SSDP_UDP.stop();
  if ( found > 0 ) {
#if DEBUG_XGEN
    Serial.print( "* SSDP Found " );
    Serial.print( found );
    Serial.println( " uPnP Devices on network" );
#endif
  } else {
#if DEBUG_XGEN
    Serial.print( "* SSDP Found no uPnP Devices on network" );
#endif
  }
  for ( t = found; t < Listsize; ++t )
    List[t] = IPAddress( 0, 0, 0, 0 ); // Fill remaining list with 0.0.0.0-IP
  return found;
}

// Parse IP in looped string "***www.xxx.yyy.zzz:1400***"
void SonosUPnP::readback_IP( IPAddress *IPa, char *buf, char pointer, char bufsize ) {
  int t, digit, decimal;
  IPAddress tmpIP( 0, 0, 0, 0 );

  for ( t = 0; t < 5; ++t ) { // turn back pointer 5 places - looped
    if ( pointer == 0 )
      pointer = 31;
    else
      pointer = pointer - 1;
  }
  digit   = 0;
  decimal = 1;
  for ( t = 0; t < 4; ++t ) {
    while ( buf[pointer] > 47 && buf[pointer] < 58 ) { // as long as digit : count on
      digit   = digit + ( buf[pointer] - 48 ) * decimal;
      decimal = decimal * 10;
      if ( pointer == 0 )
        pointer = 31; // loop pointer back
      else
        pointer = pointer - 1;
    }
    tmpIP[3 - t] = digit; // end of digit found
    decimal      = 1;
    digit        = 0;
    if ( pointer == 0 )
      pointer = 31; // loop pointer back
    else
      pointer = pointer - 1;
  }
  *IPa = tmpIP; // copy Ipadress into pointer to give back
}

#ifndef SONOS_WRITE_ONLY_MODE

void SonosUPnP::ethClient_xPath( PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize ) {
  xPath.setPath( path, pathSize );
  while ( ethClient.available() ) {
    char c = ethClient.read();
#if DEBUG_XPATH
    Serial.print( c ); /*****************/
#endif
    if ( xPath.getValue( c, resultBuffer, resultBufferSize ) )
      break;
  }
#if DEBUG_XPATH
  // Serial.println( "in the orig xpath function" ); /*****************/
#endif
}

// JV
// Modified version of xpath call: re-work XML special charactes '&lt;' to '<' and '&gt;' to '>' for correct parsing Attributes
//
void SonosUPnP::ethClient_xPath2( PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize ) {
  char c, d, e, f;
  xPath.setPath( path, pathSize );
  while ( ethClient.available() ) {
    c = ethClient.read();
    if ( c == '&' ) {
      d = ethClient.read();
      e = ethClient.read();
      f = ethClient.read();
      if ( d == 'l' && e == 't' && f == ';' ) {
        c = '<';
#if DEBUG_XPATH
        Serial.print( c ); /*****************/
#endif
        if ( xPath.getValue( c, resultBuffer, resultBufferSize ) )
          break;
      } else if ( d == 'g' && e == 't' && f == ';' ) {
        c = '>';
#if DEBUG_XPATH
        Serial.print( c ); /*****************/
#endif
        if ( xPath.getValue( c, resultBuffer, resultBufferSize ) )
          break;
      } else {
#if DEBUG_XPATH
        Serial.print( c );   /*****************/
        Serial.print( d );   /*****************/
        Serial.print( e );   /*****************/
        Serial.println( f ); /*****************/
#endif
        if ( xPath.getValue( c, resultBuffer, resultBufferSize ) )
          break;
        if ( xPath.getValue( d, resultBuffer, resultBufferSize ) )
          break;
        if ( xPath.getValue( e, resultBuffer, resultBufferSize ) )
          break;
        if ( xPath.getValue( f, resultBuffer, resultBufferSize ) )
          break;
      }
    } else {
#if DEBUG_XPATH
      Serial.print( c ); /*****************/
#endif
      if ( xPath.getValue( c, resultBuffer, resultBufferSize ) )
        break;
    }
  } // end while ethernet input loop
  convertMetaData( resultBuffer ); // Strip XML feedback from special charaters.
}

void SonosUPnP::upnpGetString( IPAddress speakerIP, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *value, PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize ) {
  if ( upnpPost( speakerIP, upnpMessageType, action_P, field, value, "", 0, 0, "" ) ) {
    xPath.reset();
    ethClient_xPath( path, pathSize, resultBuffer, resultBufferSize );
  }
  ethClient_stop();
}

bool SonosUPnP::isValidTimeFormat(const char *str) {
    int len = strlen(str);

    // Acceptable lengths: "0:00:00" (7 chars) or "00:00:00" (8 chars)
    if (len < 7 || len > 8) return false;

    // Check if the first character is a digit
    if (!isdigit(str[0])) return false;

    // Check if the second character is a digit or a colon
    if (len == 8 && !isdigit(str[1])) return false; // If length is 8, it must be a digit
    if (len == 7 && str[1] != ':') return false;    // If length is 7, it must be a colon

    // Check fixed positions for colons
    if (len == 8) {
        if (str[2] != ':' || str[5] != ':') return false;
    } else { // len == 7
        if (str[1] != ':' || str[4] != ':') return false;
    }

    // Check remaining digits
    int startIndex = (len == 8) ? 3 : 2;  // Adjust index based on format
    for (int i = startIndex; i < len; i++) {
        if (i == startIndex + 2 || i == startIndex + 5) {
            if (str[i] != ':') return false;  // Ensure colons are in the right places
        } else {
            if (!isdigit(str[i])) return false;  // Ensure digits in other places
        }
    }
    return true;
}

uint32_t SonosUPnP::getTimeInSeconds( const char *time ) {
  
  uint8_t len      = strlen( time );
  uint32_t seconds = 0;
  uint8_t dPower   = 0;
  uint8_t tPower   = 0;
  for ( int8_t i = len; i > 0; i-- ) {
    char character = time[i - 1];
    if ( character == ':' ) {
      dPower = 0;
      tPower++;
    } else if ( character >= '0' && character <= '9' ) {
      seconds += ( character - '0' ) * uiPow( 10, dPower ) * uiPow( 60, tPower );
      dPower++;
    }
  }
  return seconds;
}

uint32_t SonosUPnP::uiPow( uint16_t base, uint16_t exponent ) {
  int result = 1;
  while ( exponent ) {
    if ( exponent & 1 )
      result *= base;
    exponent >>= 1;
    base *= base;
  }
  return result;
}

uint8_t SonosUPnP::convertMedium( const char *input ) {
  if ( strcmp( input, SONOS_MEDIUM_LINEIN_VALUE ) == 0 )
    return SONOS_MEDIUM_LINEIN;
  if ( strcmp( input, SONOS_MEDIUM_NETWORK_VALUE ) == 0 )
    return SONOS_MEDIUM_NETWORK;
  return SONOS_MEDIUM_NONE;
}

uint8_t SonosUPnP::convertState( const char *input ) {
  if ( strcmp( input, SONOS_STATE_PLAYING_VALUE ) == 0 )
    return SONOS_STATE_PLAYING;
  if ( strcmp( input, SONOS_STATE_PAUSED_VALUE ) == 0 )
    return SONOS_STATE_PAUSED;
  return SONOS_STATE_STOPPED;
}

uint8_t SonosUPnP::convertPlayMode( const char *input ) {
  if ( strcmp( input, SONOS_PLAY_MODE_NORMAL_VALUE ) == 0 )
    return SONOS_PLAY_MODE_NORMAL;
  if ( strcmp( input, SONOS_PLAY_MODE_REPEAT_VALUE ) == 0 )
    return SONOS_PLAY_MODE_REPEAT;
  if ( strcmp( input, SONOS_PLAY_MODE_SHUFFLE_REPEAT_VALUE ) == 0 )
    return SONOS_PLAY_MODE_SHUFFLE_REPEAT;
  if ( strcmp( input, SONOS_PLAY_MODE_SHUFFLE_VALUE ) == 0 )
    return SONOS_PLAY_MODE_SHUFFLE;
  return SONOS_PLAY_MODE_NORMAL;
}

/*
Parse String with XML meta data on specia HTML characters

Converts:
&lt; = '<'  ascii d60
&gt; = '>' ascii d62
&apos; '\'' ascii d44
&quot; '\"' ascii d45
&amp;  '&' ascii 38

&amp; is only converted to '&' if followed by &amp, otherwise its discarded
*/
uint8_t SonosUPnP::convertMetaData( char *input ) {
  int rp = 0, wp = 0; // read pointer, write pointer
  char c, d, e, f, amp = 0;

  while ( input[rp] != 0 ) {
    c = input[rp];
    rp++;
    if ( c == '&' ) {
#ifdef DEBUG_XGEN
      Serial.print( "* XML Metadata parser found &:" );
      Serial.println( &input[rp - 1] );
#endif
      c = input[rp];
      rp++;
      d = input[rp];
      rp++;
      e = input[rp];
      rp++;
      if ( c == 'l' && d == 't' && e == ';' ) {
        input[wp] = '<';
        wp++;
      }
      if ( c == 'g' && d == 't' && e == ';' ) {
        input[wp] = '>';
        wp++;
      }
      if ( c == 'a' && d == 'p' && e == 'o' ) {
        rp++;
        rp++;
        input[wp] = '\'';
        wp++;
      } // read s read ;
      if ( c == 'q' && d == 'u' && e == 'o' ) {
        rp++;
        rp++;
        input[wp] = '\"';
        wp++;
      } // read t read ;

      if ( c == 'a' && d == 'm' && e == 'p' ) {
        if ( amp == 0 ) {
          amp       = 1;
          input[rp] = '&';
        } // found &amp; first time, set next input ';' info another '&', and loop again
        else {
          rp++;
          input[wp] = '&';
          wp++;
          amp = 0;
        } // found &amp;amp second time, print '&'
      } else
        amp = 0;
    } else {
      input[wp] = c;
      wp++;
    }
    if ( rp > 100 )
      break; // break loop if there is too much read
  }
  input[wp] = 0; // close character string
  return ( rp );
}

#endif
