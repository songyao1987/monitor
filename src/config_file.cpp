#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "config_file.hpp"

const char * ConfigFile::DefaultConfName1 = "/home/et1000/watchdog.conf";
const char * ConfigFile::DefaultConfName2 = "/home/et1000/conf/watchdog.conf";

unsigned int rebootinterval;

// 默认的cpu 类型为2410
char g_cputype_str[20] = "2410";

// 默认的规约地方版本为 Standard
char g_locale_str[20] = "Standard";

// 默认的地区配置为大连
char g_area_str[20] = "Dalian";

char g_subarea_str[20] = "Shenyang";

// 默认的基表串口波特率为 19200bps
char g_baudrate_str[20] = "19200";

// 默认不带CS配置
char g_cs_str[20] = "No";

ConfigFile::ConfigFile()
{
  PrgList = 0;
  CurrNode = 0;
}

ConfigFile::~ConfigFile()
{
  Clear();
}


void ConfigFile::Read( char const * FileName ) 
{
  FILE * fp;
  char line[160];
  char delimiter[] = " =,:\t\n";
  char tmp[160], *tmp_ptr;
  char *token;
  char *last;
  int count = 0;
  int i;

  if ( FileName == 0 || strlen( FileName ) == 0 ) {
    if ( access( DefaultConfName1, R_OK ) == 0 ) 
      FileName = DefaultConfName1;
    else 
      FileName = DefaultConfName2;
  }

  fp = fopen( FileName, "r" );

  if ( fp == 0 ) {
    fprintf( stderr, "can not open watchdog config file: %s", FileName );
    return;
  }


  //to set default reboot interval -- 10days
  rebootinterval = 10 * 3600 * 24;
  
  fseek( fp, 0L, SEEK_SET );

  while( !feof( fp ) ) {
    struct ProgramList *curr = NULL;

    fgets( line, sizeof( line ), fp );
    token = strtok_r( line, delimiter, &last );

    if ( token == 0 || token[0] == '#' ) {
      continue;
    }

    if ( ( strcmp( token, "reboot" ) != 0 ) && 
	 ( strcmp( token, "rerun" ) != 0 ) && 
	 ( strcmp( token, "once" ) != 0 ) &&
	 ( strcmp( token, "rebootinterval" ) != 0 ) &&
         ( strcmp( token, "locale" ) != 0 ) &&
         ( strcmp( token, "cputype" ) != 0 ) &&
         ( strcmp( token, "area" ) != 0 ) &&
         ( strcmp( token, "subarea" ) != 0 ) &&
         ( strcmp( token, "cssupport" ) != 0 ) &&
         ( strcmp( token, "baudrate" ) != 0)) {
      continue;
    }

    if ( strcmp( token, "rebootinterval" ) == 0 ) {
      unsigned int hours;
      token = strtok_r( 0, delimiter, &last );
      hours = atoi( token );
      hours = ( hours < 240 )? hours : 240;
      rebootinterval = hours * 3600;
      continue;
    }

    if ( strcmp( token, "locale" ) == 0 ) {
      token = strtok_r( 0, delimiter, &last );
      if ( strlen ( token ) > 0 ) {
	strncpy( g_locale_str, token, 20 );
	g_locale_str[19] = 0;
      }
      continue;
    }
    
    if ( strcmp( token, "cssupport" ) == 0 ) {
      token = strtok_r( 0, delimiter, &last );
      if ( strlen ( token ) > 0 ) {
	strncpy( g_cs_str, token, 20 );
	g_cs_str[19] = 0;
      }
      continue;
    }
    
    if ( strcmp( token, "cputype" ) == 0 ) {
      token = strtok_r( 0, delimiter, &last );
      if ( strlen ( token ) > 0 ) {
	strncpy( g_cputype_str, token, 20 );
	g_cputype_str[19] = 0;
      }
      continue;
    }   
    
    if ( strcmp( token, "area" ) == 0 ) {
      token = strtok_r( 0, delimiter, &last );
      if ( strlen ( token ) > 0 ) {
	strncpy( g_area_str, token, 20 );
	g_area_str[19] = 0;
      }
      continue;
    }
    
    if ( strcmp( token, "subarea" ) == 0 ) {
      token = strtok_r( 0, delimiter, &last );
      if ( strlen ( token ) > 0 ) {
	strncpy( g_subarea_str, token, 20 );
	g_subarea_str[19] = 0;
      }
      continue;
    }
    
    if ( strcmp( token, "baudrate" ) == 0 ) {
      token = strtok_r( 0, delimiter, &last );
      if ( strlen ( token ) > 0 ) {
	strncpy( g_baudrate_str, token, 20 );
	g_baudrate_str[19] = 0;
      }
      continue;
    }
 
    curr = ( struct ProgramList * )malloc( sizeof( struct ProgramList ) );
    if(curr == NULL)
    {
        printf("内存不足\n");
        exit(0);
    }
    /* to set program run parameters */
    if ( strcmp( token, "reboot" ) == 0 ) {
      curr->node.mode = REBOOT;
    }
    else if ( strcmp( token, "rerun" ) == 0 ){
      curr->node.mode = RERUN;
    }
    else {
      curr->node.mode = ONCE;
    }

    token = strtok_r( 0, delimiter, &last );
    if ( token == 0 ) {
      free( curr );
      continue;
    }
    curr->node.path = strdup( token );

    tmp_ptr = tmp;
    for ( i = 0; i < 10; i ++ ) {
      token = strtok_r( 0, delimiter, &last );
      if ( token == 0 ) {
	curr->node.param[i] = 0;
	break;
      }
      curr->node.param[i] = strdup( token );
    }

    count ++;
    curr->next = PrgList;
    PrgList = curr;

  } /* while( !feof() ) */

  fclose( fp );
  return;
}

struct Program * ConfigFile::FirstProg( )
{  
  CurrNode = PrgList;

  return CurrProg();
}

struct Program * ConfigFile::NextProg( )
{
  if ( CurrNode == 0 ) {
    return 0;
  }

  CurrNode = CurrNode->next;

  if ( CurrNode == 0 ) {
    return 0;
  }

  return CurrProg();

}

struct Program * ConfigFile::CurrProg()
{
  if ( CurrNode == 0 ) {
    return 0;
  }

  return &( CurrNode->node );
}

void ConfigFile::Clear()
{
  int i;
  struct ProgramList *tmp = PrgList;

  while( PrgList != 0 ) {
    for ( i = 0; i < 10; i ++ ) {
      if ( PrgList->node.param[i] == 0 ) {
	break;
      }
      free( PrgList->node.param[i] );
    }
    free( PrgList->node.path );
    tmp = PrgList ;
    PrgList = PrgList->next;
    free( tmp );
  }

  CurrNode = 0;
  PrgList = 0;
}

