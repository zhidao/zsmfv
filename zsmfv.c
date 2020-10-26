/*
 * zsmfv - Z's SMF Viewer ver1.4 by Zhidao
 *
 * 2001. 8.26. Created.
 * 2007.12.15. Last Updated.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "1.5"

#define ZSMFVERROR( msg ) fprintf( stderr, "ERROR: %s.\n", msg )
#define ZSMFVWARN( msg )  fprintf( stderr, "WARNING: %s.\n", msg )

typedef unsigned char BYTE;

static BYTE quiet = 0; /* quiet mode flag */

#define PRINTF if( !quiet ) printf

static unsigned long timebase;       /* timebase */
static unsigned long tempo = 500000; /* = 120[crotchet/min] */
static unsigned long tracktime = 0;  /* total time [msec] of a track */
static unsigned long totaltime = 0;  /* total time [msec] */

/* ************************************************************* *
 * initial & terminate operation
 * ************************************************************* */

/* write an usage of zsmfv to stderr */
void zsmfvUsage(void)
{
  fprintf( stderr, "Usage: zsmfv <options> file\n" );
  fprintf( stderr, " file    SMF file name\n" );
  fprintf( stderr, "options\n" );
  fprintf( stderr, " -h      showing this message\n" );
  fprintf( stderr, " -t      showing only total time\n" );
  exit( 1 );
}

/* output version */
void zsmfvVersion(void)
{
  PRINTF( "zsmfv ver%s (C)Zhidao, all right reserved\n", VERSION );
}

/* start zsmfv - open SMF */
FILE *zsmfvStart(int argc, char *argv[])
{
  FILE *fp;
  char *fn, filename[BUFSIZ];

  /* output version */
  zsmfvVersion();

  if( argc < 2 || !strcmp( argv[1], "-h" ) )
    zsmfvUsage();
  if( !strcmp( argv[1], "-t" ) ){
    quiet = 1;
    fn = argv[2];
  } else
    fn = argv[1];
  if( fn == NULL ) zsmfvUsage();

  fp = fopen( fn, "rb" );
  if( fp == NULL ){
    sprintf( filename, "%s.mid", fn );
    fp = fopen( filename, "rb" );
    if( fp == NULL ){
      ZSMFVERROR( "cannot open file" );
      exit( 1 );
    }
  }
  return fp;
}

/* terminate zsmfv - close SMF */
void zsmfvTerminate(FILE *fp)
{
  fclose( fp );
}

/* ************************************************************* *
 * misc
 * ************************************************************* */

unsigned long bytesize = 0;

/* read a byte from SMF */
BYTE zsmfvReadByte(FILE *fp)
{
  bytesize++;
  return feof( fp ) ? 0 : fgetc( fp );
}

/* put back a byte to SMF */
void zsmfvWriteBackByte(FILE *fp)
{
  bytesize--;
  fseek( fp, -1, SEEK_CUR );
}

static BYTE buf[BUFSIZ];

/* read bytes from SMF */
BYTE *zsmfvReadData(FILE *fp, int size)
{
  BYTE *cp;

  for( cp=buf; size>0; size-- )
    *cp++ = zsmfvReadByte( fp );
  *cp = '\0';
  return buf;
}

/* convert bytes to unsigned long value */
unsigned long zsmfvBytesToValue(BYTE *bytes, int size)
{
  int i;
  unsigned long value = 0;

  for( i=0; i<size; i++ ){
    value <<= 8;
    value |= bytes[i];
  }
  return value;
}

/* read variable-size-value according to SMF rule */
unsigned long zsmfvReadVariableSizeValue(FILE *fp)
{
  BYTE bt;
  unsigned long value = 0;

  do {
    value <<= 7;
    bt = zsmfvReadByte( fp );
    value |= bt & 0x7f;
  } while( bt & 0x80 );
  return value;
}

/* convert hexadecimal value to a string */
char *zsmfvHex(BYTE val)
{
  static char hex_str[3];

  sprintf( hex_str, "%1X%1X", (val&0xf0)>>4, val&0xf );
  return hex_str;
}

/* convert note number to note name */
char *zsmfvNoteName(BYTE note)
{
  static char *notename[] = {
    "C", "C+", "D", "D+", "E", "F", "F+", "G", "G+", "A", "A+", "B"
  };
  static char note_str[4];

  note_str[0] = ( (int)note / 12 ) + '0';
  strcpy( note_str+1, notename[(int)note % 12 ] );
  return note_str;
}

/* output total time */
void zsmfvTotalTime(void)
{
  unsigned long min, sec, msec;

  sec = totaltime / 1000;
  msec = totaltime - sec * 1000;
  min = sec / 60;
  sec -= min * 60;
  PRINTF( "totaltime = %ld:%ld.%ld\n", min, sec, msec );
}

/* ************************************************************* *
 * read header chunk
 * ************************************************************* */

/* read MThd charactor sequence */
void zsmfvReadMThd(FILE *fp)
{
  if( strncmp( (const char *)zsmfvReadData( fp, 4 ), "MThd", 4 ) ){
    ZSMFVERROR( "file is proberbly broken or not SMF" );
    ZSMFVERROR( "cannot find header chunk" );
    exit( 1 );
  }
  PRINTF( "MThd: +++ header chunk +++\n" );
}

/* read header chunk size */
unsigned long zsmfvReadMThdSize(FILE *fp)
{
  unsigned long size;

  size = zsmfvBytesToValue( zsmfvReadData( fp, 4 ), 4 );
  PRINTF( " size = %lu\n", size );
  return size;
}

/* read format of SMF */
unsigned long zsmfvReadFormat(FILE *fp)
{
  unsigned long format;

  format = zsmfvBytesToValue( zsmfvReadData( fp, 2 ), 2 );
  PRINTF( " format %lu\n", format );
  switch( format ){
  case 0: case 1: case 2: break;
  default: ZSMFVERROR( "unknown format" ); exit( 1 );
  }
  return format;
}

/* read number of tracks */
unsigned long zsmfvReadTrackNum(FILE *fp)
{
  unsigned long num;

  num = zsmfvBytesToValue( zsmfvReadData( fp, 2 ), 2 );
  PRINTF( " track number = %lu\n", num );
  return num;
}

/* read time base of SMF */
unsigned long zsmfvReadTimebase(FILE *fp)
{
  timebase = zsmfvBytesToValue( zsmfvReadData( fp, 2 ), 2 );
  PRINTF( " time base = %lu\n", timebase );
  return timebase;
}

/* read header chunk of SMF */
unsigned long zsmfvReadHeaderChunk(FILE *fp)
{
  unsigned long format, tracknum;

  zsmfvReadMThd( fp );
  zsmfvReadMThdSize( fp );
  format = zsmfvReadFormat( fp );
  tracknum = zsmfvReadTrackNum( fp );
  if( format == 0 && tracknum != 1 )
    ZSMFVWARN( "invalid track number for format0" );
  zsmfvReadTimebase( fp );
  return tracknum;
}

/* ************************************************************* *
 * MIDI event
 * ************************************************************* */

/* read note-off event */
void zsmfvNoteOff(FILE *fp, BYTE ch)
{
  int note, velocity;

  note = zsmfvReadByte( fp );
  velocity = zsmfvReadByte( fp );
  PRINTF( " [%d]NoteOff: note=%s velocity=%d\n",
    ch, zsmfvNoteName(note), velocity );
}

/* read note-on event */
void zsmfvNoteOn(FILE *fp, BYTE ch)
{
  int note, velocity;

  note = zsmfvReadByte( fp );
  velocity = zsmfvReadByte( fp );
  PRINTF( " [%d]NoteOn: note=%s velocity=%d\n",
    ch, zsmfvNoteName(note), velocity );
}

/* read polyphonic-key-pressure event */
void zsmfvPolyphonicKeyPressure(FILE *fp, BYTE ch)
{
  int note, pressure;

  note = zsmfvReadByte( fp );
  pressure = zsmfvReadByte( fp );
  PRINTF( " [%d]PolyphonicKeyPressure: note=%s pressure=%d\n",
    ch, zsmfvNoteName(note), pressure );
}

/* read bank-select event */
void zsmfvBankSelect(FILE *fp, BYTE ch)
{
  BYTE id;

  id = zsmfvReadByte( fp );
  PRINTF( " [%d]BankSelect: bank=%s\n", ch, zsmfvHex( id ) );
}

/* read control-change event */
void zsmfvControlChange(FILE *fp, BYTE ch)
{
  BYTE id, val;

  id = zsmfvReadByte( fp );
  if( id == 0x00 || id == 0x20 ){
    zsmfvBankSelect( fp, ch );
    return;
  }
  val = zsmfvReadByte( fp );
  PRINTF( " [%d]ControlChange: %s ", ch, zsmfvHex( id ) );
  PRINTF( "value=%s\n", zsmfvHex( val ) );
}

/* read program-change event */
void zsmfvProgramChange(FILE *fp, BYTE ch)
{
  BYTE pn;

  pn = zsmfvReadByte( fp );
  PRINTF( " [%d]ProgramChange: program=%s\n", ch, zsmfvHex( pn ) );
}

/* read channel-presure event */
void zsmfvChannelPressure(FILE *fp, BYTE ch)
{
  int pressure;

  pressure = zsmfvReadByte( fp );
  PRINTF( " [%d]ChannelPressure: pressure=%d\n", ch, pressure );
}

/* read pitch-bend event */
void zsmfvPitchBend(FILE *fp, BYTE ch)
{
  BYTE msb, lsb;

  lsb = zsmfvReadByte( fp );
  msb = zsmfvReadByte( fp );
  PRINTF( " [%d]PitchBend: value=%s\n", ch, zsmfvHex( msb << 7 | lsb ) );
}

/* ************************************************************* *
 * system exclusive
 * ************************************************************* */

/* read system-exclusive event */
void zsmfvSystemExclusive(FILE *fp)
{
  int size, i;
  BYTE val;

  size = zsmfvReadByte( fp );
  PRINTF( " SystemExclusive:" );
  for( i=0; i<size; i++ ){
    val = zsmfvReadByte( fp );
    PRINTF( " %s", zsmfvHex( val ) );
  }
  PRINTF( "\n" );
}

/* ************************************************************* *
 * meta event
 * ************************************************************* */

/* read meta event - sequence no. */
void zsmfvMetaSequenceNo(FILE *fp)
{
  PRINTF( "SequenceNo." );
  if( zsmfvReadByte( fp ) != 2 ){
    PRINTF( "?\n" );
    ZSMFVERROR( "invalid data size" );
    exit( 1 );
  }
  zsmfvReadData( fp, 2 );
  PRINTF( "%lu\n", zsmfvBytesToValue( buf, 2 ) );
}

/* read meta event - text type */
void zsmfvMetaTextTypeEvent(FILE *fp, char type[])
{
  unsigned long length;

  length = zsmfvReadVariableSizeValue( fp );
  zsmfvReadData( fp, length );
  PRINTF( "%s: %s\n", type, buf );
}

#define zsmfvMetaTextEvent( fp ) zsmfvMetaTextTypeEvent( fp, "Text" )
#define zsmfvMetaCopyRight( fp ) zsmfvMetaTextTypeEvent( fp, "Copyright" )
#define zsmfvMetaTrackName( fp ) zsmfvMetaTextTypeEvent( fp, "TrackName" )
#define zsmfvMetaInstrument( fp ) zsmfvMetaTextTypeEvent( fp, "Instrument" )
#define zsmfvMetaLyrics( fp ) zsmfvMetaTextTypeEvent( fp, "Lyrics" )
#define zsmfvMetaMarker( fp ) zsmfvMetaTextTypeEvent( fp, "Marker" )
#define zsmfvMetaCuePoint( fp ) zsmfvMetaTextTypeEvent( fp, "CuePoint" )
#define zsmfvMetaOther( fp ) zsmfvMetaTextTypeEvent( fp, "(event)" )

/* read meta event - MIDI channel prefix */
void zsmfvMetaMIDIChannelPrefix(FILE *fp)
{
  int ch;

  PRINTF( "MIDI channel prefix [" );
  if( zsmfvReadByte( fp ) != 1 ){
    PRINTF( "?]\n" );
    ZSMFVERROR( "invalid data size" );
    exit( 1 );
  }
  ch = zsmfvReadByte( fp );
  PRINTF( "%d]\n", ch );
}

/* read meta event - MIDI port set */
void zsmfvMetaMIDIPortSet(FILE *fp)
{
  int port;

  PRINTF( "MIDI port set [" );
  if( zsmfvReadByte( fp ) != 1 ){
    PRINTF( "?]\n" );
    ZSMFVERROR( "invalid data size" );
    exit( 1 );
  }
  port = zsmfvReadByte( fp );
  PRINTF( "%c]\n", port + 'A' );
}

/* read meta event - end of track */
void zsmfvMetaEOT(FILE *fp)
{
  PRINTF( "<EOT>\n" );
  if( zsmfvReadByte( fp ) != 0 ){
    ZSMFVERROR( "invalid data size" );
    exit( 1 );
  }
}

/* read meta event - set tempo */
void zsmfvMetaSetTempo(FILE *fp)
{
  PRINTF( "SetTempo = " );
  if( zsmfvReadByte( fp ) != 3 ){
    PRINTF( "?\n" );
    ZSMFVERROR( "invalid data size" );
    exit( 1 );
  }
  tempo = zsmfvBytesToValue( zsmfvReadData( fp, 3 ), 3 );
  /* tempo[microsec/crotchet] */
  PRINTF( "crotchet = %lu\n", 60000000/tempo );
}

/* read meta event - SMTPE offset */
void zsmfvMetaSMTPEOffset(FILE *fp)
{
  int hour, minute, second, frame, fruction;

  PRINTF( "SMTPE Offset: " );
  if( zsmfvReadByte( fp ) != 5 ){
    PRINTF( "?\n" );
    ZSMFVERROR( "invalid data size" );
    exit( 1 );
  }
  hour = zsmfvReadByte( fp );
  minute = zsmfvReadByte( fp );
  second = zsmfvReadByte( fp );
  frame = zsmfvReadByte( fp );
  fruction = zsmfvReadByte( fp );
  switch( hour >> 5 ){
  case 0: PRINTF( "24[frame/s] " ); break;
  case 1: PRINTF( "25[frame/s] " ); break;
  case 2: PRINTF( "30[frame/s(drop)] " ); break;
  case 3: PRINTF( "30[frame/s(non-drop)] " ); break;
  default:
    PRINTF( "?\n" );
    ZSMFVERROR( "invalid data size" );
    exit( 1 );
  }
  PRINTF( "%d[hour] %d[minute] %d[second] ", hour&0x1f, minute, second );
  PRINTF( "%d+%d/100[frame]\n", frame, fruction );
}

/* read meta event - beat */
void zsmfvMetaBeat(FILE *fp)
{
  int d, e, c, s;

  PRINTF( "Beat = " );
  if( zsmfvReadByte( fp ) != 4 ){
    PRINTF( "?\n" );
    ZSMFVERROR( "invalid data size" );
    exit( 1 );
  }
  d = zsmfvReadByte( fp );
  e = zsmfvReadByte( fp );
  c = zsmfvReadByte( fp );
  s = zsmfvReadByte( fp );
  PRINTF( "(%d/%d) clock=%d, %d[32/24clock]\n", d, 2<<(e-1), c, s );
}

/* read meta event - key */
void zsmfvMetaKey(FILE *fp)
{
  char s;

  PRINTF( "Key = " );
  if( zsmfvReadByte( fp ) != 2 ){
    PRINTF( "?\n" );
    ZSMFVERROR( "invalid data size" );
    exit( 1 );
  }
  s = zsmfvReadByte( fp );
  switch( zsmfvReadByte( fp ) ){
  case 0x0: PRINTF( "Major " ); break;
  case 0x1: PRINTF( "Minor " ); break;
  default:
    PRINTF( "?\n" );
    ZSMFVERROR( "invalid code specification" );
    exit( 1 );
  }
  if( s >= 0 ){
    PRINTF( "(# x %d)\n", s );
  } else{
    PRINTF( "(b x %d)\n", s );
  }
}

/* read meta event - unknown type */
void zsmfvMetaUnknown(FILE *fp)
{
  int i, size;
  BYTE val;

  size = zsmfvReadByte( fp );
  for( i=0; i<size; i++ ){
    val = zsmfvReadByte( fp );
    PRINTF( " %s", zsmfvHex( val ) );
  }
  PRINTF( "\n" );
}

/* read meta event */
void zsmfvMetaEvent(FILE *fp)
{
  BYTE type;

  PRINTF( " MetaEvent-" );
  type = zsmfvReadByte( fp );
  switch( type ){
  case 0x00: zsmfvMetaSequenceNo( fp );        break;
  case 0x01: zsmfvMetaTextEvent( fp );         break;
  case 0x02: zsmfvMetaCopyRight( fp );         break;
  case 0x03: zsmfvMetaTrackName( fp );         break;
  case 0x04: zsmfvMetaInstrument( fp );        break;
  case 0x05: zsmfvMetaLyrics( fp );            break;
  case 0x06: zsmfvMetaMarker( fp );            break;
  case 0x07: zsmfvMetaCuePoint( fp );          break;
  case 0x20: zsmfvMetaMIDIChannelPrefix( fp ); break;
  case 0x21: zsmfvMetaMIDIPortSet( fp );       break;
  case 0x2F: zsmfvMetaEOT( fp );               break;
  case 0x51: zsmfvMetaSetTempo( fp );          break;
  case 0x54: zsmfvMetaSMTPEOffset( fp );       break;
  case 0x58: zsmfvMetaBeat( fp );              break;
  case 0x59: zsmfvMetaKey( fp );               break;
  case 0x7F: zsmfvMetaOther( fp );             break;
  default:
    PRINTF( "(unknown %s):", zsmfvHex( type ) );
    zsmfvMetaUnknown( fp );
  }
}

/* ************************************************************* *
 * read track chunk
 * ************************************************************* */

/* read MTrk charactor sequence */
void zsmfvReadMTrk(FILE *fp)
{
  if( strncmp( (const char *)zsmfvReadData( fp, 4 ), "MTrk", 4 ) ){
    ZSMFVERROR( "file is proberbly broken or not SMF" );
    ZSMFVERROR( "cannot find track chunk" );
    exit( 1 );
  }
  PRINTF( "MTrk: +++ track chunk +++\n" );
}

/* read track chunk size */
unsigned long zsmfvReadMTrkSize(FILE *fp)
{
  unsigned long size;

  size = zsmfvBytesToValue( zsmfvReadData( fp, 4 ), 4 );
  PRINTF( " size = %lu\n", size );
  return size;
}

/* read delta time for each event */
void zsmfvReadDeltaTime(FILE *fp)
{
  unsigned long dt;

  dt = zsmfvReadVariableSizeValue( fp );
  PRINTF( " dt=%3lu:", dt );
  /* total time increment */
  tracktime += tempo * dt / timebase / 1000;
}

/* read each event */
void zsmfvReadEvent(FILE *fp)
{
  static BYTE old_event_id = 0;
  BYTE event_id, ch;

  event_id = zsmfvReadByte( fp );
 RETRY:
  ch = event_id & 0xf;
  switch( event_id >> 4 ){
  case 0x8: zsmfvNoteOff( fp, ch );               break;
  case 0x9: zsmfvNoteOn( fp, ch );                break;
  case 0xa: zsmfvPolyphonicKeyPressure( fp, ch ); break;
  case 0xb: zsmfvControlChange( fp, ch );         break;
  case 0xc: zsmfvProgramChange( fp, ch );         break;
  case 0xd: zsmfvChannelPressure( fp, ch );       break;
  case 0xe: zsmfvPitchBend( fp, ch );             break;
  case 0xf:
    switch( ch ){
    case 0x0:
    case 0x7: zsmfvSystemExclusive( fp ); break;
    case 0xf: zsmfvMetaEvent( fp ); break;
    default: goto UNKNOWN_EVENT;
    }
    break;
  default:
    event_id = old_event_id;
    zsmfvWriteBackByte( fp );
    goto RETRY;
  }
  old_event_id = event_id; /* for running status */
  return;

 UNKNOWN_EVENT:
  fprintf( stderr, " event = %s\n", zsmfvHex( event_id ) );
  ZSMFVERROR( "unknown event type" );
  exit( 1 );
}

/* read track chunk */
unsigned long zsmfvReadTrackChunk(FILE *fp)
{
  unsigned long size;

  tracktime = 0;
  zsmfvReadMTrk( fp );
  size = zsmfvReadMTrkSize( fp );
  bytesize = 0;
  while( bytesize < size ){
    if( feof( fp ) ){
      ZSMFVERROR( "invalid track size" );
      exit( 1 );
    }
    zsmfvReadDeltaTime( fp );
    zsmfvReadEvent( fp );
  }
  PRINTF( "read %lu/%lu bytes\n", size, bytesize );
  /* totaltime of the score is the total time of the longest track */
  if( tracktime > totaltime ) totaltime = tracktime;
  return size;
}

/* ************************************************************* *
 * main
 * ************************************************************* */

int main(int argc, char *argv[])
{
  FILE *fp;
  unsigned tracknum, i;

  /* open SMF */
  fp = zsmfvStart( argc, argv );
  /* read header chunk */
  tracknum = zsmfvReadHeaderChunk( fp );
  /* read track chunk */
  for( i=0; i<tracknum; i++ )
    zsmfvReadTrackChunk( fp );
  /* output total time */
  zsmfvTotalTime();
  /* termination */
  zsmfvTerminate( fp );
  return 0;
}
