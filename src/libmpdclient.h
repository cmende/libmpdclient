/* libmpdclient
   (c) 2003-2008 The Music Player Daemon Project
   This project's homepage is: http://www.musicpd.org

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   - Neither the name of the Music Player Daemon nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LIBMPDCLIENT_H
#define LIBMPDCLIENT_H

#include "song.h"

#ifdef WIN32
#  define __W32API_USE_DLLIMPORT__ 1
#endif

#include <stddef.h>
#include <sys/time.h>
#include <stdarg.h>
#ifdef MPD_GLIB
#include <glib.h>
#endif

#define MPD_WELCOME_MESSAGE	"OK MPD "

enum mpd_error {
	/** no error */
	MPD_ERROR_SUCCESS = 0,

	/** timeout trying to talk to mpd */
	MPD_ERROR_TIMEOUT = 10,

	/** system error */
	MPD_ERROR_SYSTEM,

	/** unknown host */
	MPD_ERROR_UNKHOST,

	/** problems connecting to port on host */
	MPD_ERROR_CONNPORT,

	/** mpd not running on port at host */
	MPD_ERROR_NOTMPD,

	/** no response on attempting to connect */
	MPD_ERROR_NORESPONSE,

	/** error sending command */
	MPD_ERROR_SENDING,

	/** connection closed by mpd */
	MPD_ERROR_CONNCLOSED,

	/** ACK returned! */
	MPD_ERROR_ACK,

	/** Buffer was overrun! */
	MPD_ERROR_BUFFEROVERRUN,
};

#define MPD_ERROR_AT_UNK	-1

enum mpd_ack {
	MPD_ACK_ERROR_UNK = -1,

	MPD_ACK_ERROR_NOT_LIST = 1,
	MPD_ACK_ERROR_ARG = 2,
	MPD_ACK_ERROR_PASSWORD = 3,
	MPD_ACK_ERROR_PERMISSION = 4,
	MPD_ACK_ERROR_UNKNOWN_CMD = 5,

	MPD_ACK_ERROR_NO_EXIST = 50,
	MPD_ACK_ERROR_PLAYLIST_MAX = 51,
	MPD_ACK_ERROR_SYSTEM = 52,
	MPD_ACK_ERROR_PLAYLIST_LOAD = 53,
	MPD_ACK_ERROR_UPDATE_ALREADY = 54,
	MPD_ACK_ERROR_PLAYER_SYNC = 55,
	MPD_ACK_ERROR_EXIST = 56,
};

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mpd_TagItems
{
	MPD_TAG_ITEM_ARTIST,
	MPD_TAG_ITEM_ALBUM,
	MPD_TAG_ITEM_TITLE,
	MPD_TAG_ITEM_TRACK,
	MPD_TAG_ITEM_NAME,
	MPD_TAG_ITEM_GENRE,
	MPD_TAG_ITEM_DATE,
	MPD_TAG_ITEM_COMPOSER,
	MPD_TAG_ITEM_PERFORMER,
	MPD_TAG_ITEM_COMMENT,
	MPD_TAG_ITEM_DISC,
	MPD_TAG_ITEM_FILENAME,
	MPD_TAG_ITEM_ANY,
	MPD_TAG_NUM_OF_ITEM_TYPES
} mpd_TagItems;

extern const char *const mpdTagItemKeys[MPD_TAG_NUM_OF_ITEM_TYPES];

/* internal stuff don't touch this struct */
typedef struct _mpd_ReturnElement {
	char * name;
	char * value;
} mpd_ReturnElement;

enum {
	/** song database has been updated*/
	IDLE_DATABASE = 0x1,

	/** a stored playlist has been modified, created, deleted or
	    renamed */
	IDLE_STORED_PLAYLIST = 0x2,

	/** the current playlist has been modified */
	IDLE_PLAYLIST = 0x4,

	/** the player state has changed: play, stop, pause, seek, ... */
	IDLE_PLAYER = 0x8,

	/** the volume has been modified */
	IDLE_MIXER = 0x10,

	/** an audio output device has been enabled or disabled */
	IDLE_OUTPUT = 0x20,

	/** options have changed: crossfade, random, repeat, ... */
	IDLE_OPTIONS = 0x40,
};

/* mpd_Connection
 * holds info about connection to mpd
 * use error, and errorStr to detect errors
 */
typedef struct _mpd_Connection {
	/* use this to check the version of mpd */
	int version[3];
	/* IMPORTANT, you want to get the error messages from here */
	char errorStr[512];
	enum mpd_ack errorCode;
	int errorAt;
	/* this will be set to MPD_ERROR_* if there is an error, 0 if not */
	enum mpd_error error;
	/* DON'T TOUCH any of the rest of this stuff */
	int sock;
	char buffer[16384];
	size_t buflen;
	size_t bufstart;
	int doneProcessing;
	int listOks;
	int doneListOk;
	int commandList;
	mpd_ReturnElement * returnElement;
	struct timeval timeout;
	char *request;
	int idle;
	void (*notify_cb) (struct _mpd_Connection *connection, unsigned flags, void *userdata);
	void (*startIdle) (struct _mpd_Connection *connection);
	void (*stopIdle) (struct _mpd_Connection *connection);
	void *userdata;
#ifdef MPD_GLIB
        int source_id;
#endif
} mpd_Connection;

typedef void (*mpd_NotificationCb) (mpd_Connection *connection, unsigned flags, void *userdata);

/* mpd_newConnection
 * use this to open a new connection
 * you should use mpd_closeConnection, when your done with the connection,
 * even if an error has occurred
 * _timeout_ is the connection timeout period in seconds
 */
mpd_Connection * mpd_newConnection(const char * host, int port, float timeout);

void mpd_setConnectionTimeout(mpd_Connection * connection, float timeout);

/* mpd_closeConnection
 * use this to close a connection and free'ing subsequent memory
 */
void mpd_closeConnection(mpd_Connection * connection);

/* mpd_clearError
 * clears error
 */
void mpd_clearError(mpd_Connection * connection);

/* STATUS STUFF */

/* use these with status.state to determine what state the player is in */
#define MPD_STATUS_STATE_UNKNOWN	0
#define MPD_STATUS_STATE_STOP		1
#define MPD_STATUS_STATE_PLAY		2
#define MPD_STATUS_STATE_PAUSE		3

/* us this with status.volume to determine if mpd has volume support */
#define MPD_STATUS_NO_VOLUME		-1

/* mpd_Status
 * holds info return from status command
 */
typedef struct mpd_Status {
	/* 0-100, or MPD_STATUS_NO_VOLUME when there is no volume support */
	int volume;
	/* 1 if repeat is on, 0 otherwise */
	int repeat;
	/* 1 if random is on, 0 otherwise */
	int random;
	/* playlist length */
	int playlistLength;
	/* playlist, use this to determine when the playlist has changed */
	long long playlist;
	/* use with MPD_STATUS_STATE_* to determine state of player */
	int state;
	/* crossfade setting in seconds */
	int crossfade;
	/* if a song is currently selected (always the case when state is
	 * PLAY or PAUSE), this is the position of the currently
	 * playing song in the playlist, beginning with 0
	 */
	int song;
	/* Song ID of the currently selected song */
	int songid;
	/* time in seconds that have elapsed in the currently playing/paused
	 * song
	 */
	int elapsedTime;
	/* length in seconds of the currently playing/paused song */
	int totalTime;
	/* current bit rate in kbs */
	int bitRate;
	/* audio sample rate */
	unsigned int sampleRate;
	/* audio bits */
	int bits;
	/* audio channels */
	int channels;
	/* 1 if mpd is updating, 0 otherwise */
	int updatingDb;
	/* error */
	char * error;
} mpd_Status;

void mpd_sendStatusCommand(mpd_Connection * connection);

/* mpd_getStatus
 * returns status info, be sure to free it with mpd_freeStatus()
 * call this after mpd_sendStatusCommand()
 */
mpd_Status * mpd_getStatus(mpd_Connection * connection);

/* mpd_freeStatus
 * free's status info malloc'd and returned by mpd_getStatus
 */
void mpd_freeStatus(mpd_Status * status);

typedef struct _mpd_Stats {
	int numberOfArtists;
	int numberOfAlbums;
	int numberOfSongs;
	unsigned long uptime;
	unsigned long dbUpdateTime;
	unsigned long playTime;
	unsigned long dbPlayTime;
} mpd_Stats;

typedef struct _mpd_SearchStats {
	int numberOfSongs;
	unsigned long playTime;
} mpd_SearchStats;

void mpd_sendStatsCommand(mpd_Connection * connection);

mpd_Stats * mpd_getStats(mpd_Connection * connection);

void mpd_freeStats(mpd_Stats * stats);

mpd_SearchStats * mpd_getSearchStats(mpd_Connection * connection);

void mpd_freeSearchStats(mpd_SearchStats * stats);

/* DIRECTORY STUFF */

/* mpd_Directory
 * used to store info fro directory (right now that just the path)
 */
typedef struct _mpd_Directory {
	char * path;
} mpd_Directory;

/* mpd_newDirectory
 * allocates memory for a new directory
 * use mpd_freeDirectory to free this memory
 */
mpd_Directory * mpd_newDirectory(void);

/* mpd_freeDirectory
 * used to free memory allocated with mpd_newDirectory, and it frees
 * path of mpd_Directory, so be careful
 */
void mpd_freeDirectory(mpd_Directory * directory);

/* mpd_directoryDup
 * works like strdup, but for mpd_Directory
 */
mpd_Directory * mpd_directoryDup(const mpd_Directory * directory);

/* PLAYLISTFILE STUFF */

/* mpd_PlaylistFile
 * stores info about playlist file returned by lsinfo
 */
typedef struct _mpd_PlaylistFile {
	char * path;
} mpd_PlaylistFile;

/* mpd_newPlaylistFile
 * allocates memory for new mpd_PlaylistFile, path is set to NULL
 * free this memory with mpd_freePlaylistFile
 */
mpd_PlaylistFile * mpd_newPlaylistFile(void);

/* mpd_freePlaylist
 * free memory allocated for freePlaylistFile, will also free
 * path, so be careful
 */
void mpd_freePlaylistFile(mpd_PlaylistFile * playlist);

/* mpd_playlistFileDup
 * works like strdup, but for mpd_PlaylistFile
 */
mpd_PlaylistFile * mpd_playlistFileDup(const mpd_PlaylistFile * playlist);

/* INFO ENTITY STUFF */

/* the type of entity returned from one of the commands that generates info
 * use in conjunction with mpd_InfoEntity.type
 */
#define MPD_INFO_ENTITY_TYPE_DIRECTORY		0
#define MPD_INFO_ENTITY_TYPE_SONG		1
#define MPD_INFO_ENTITY_TYPE_PLAYLISTFILE	2

/* mpd_InfoEntity
 * stores info on stuff returned info commands
 */
typedef struct mpd_InfoEntity {
	/* the type of entity, use with MPD_INFO_ENTITY_TYPE_* to determine
	 * what this entity is (song, directory, etc...)
	 */
	int type;
	/* the actual data you want, mpd_Song, mpd_Directory, etc */
	union {
		mpd_Directory * directory;
		struct mpd_song *song;
		mpd_PlaylistFile * playlistFile;
	} info;
} mpd_InfoEntity;

mpd_InfoEntity * mpd_newInfoEntity(void);

void mpd_freeInfoEntity(mpd_InfoEntity * entity);

/* INFO COMMANDS AND STUFF */

/* use this function to loop over after calling Info/Listall functions */
mpd_InfoEntity * mpd_getNextInfoEntity(mpd_Connection * connection);

/* fetches the currently seeletect song (the song referenced by status->song
 * and status->songid*/
void mpd_sendCurrentSongCommand(mpd_Connection * connection);

/* songNum of -1, means to display the whole list */
void mpd_sendPlaylistInfoCommand(mpd_Connection * connection, int songNum);

/* songId of -1, means to display the whole list */
void mpd_sendPlaylistIdCommand(mpd_Connection * connection, int songId);

/* use this to get the changes in the playlist since version _playlist_ */
void mpd_sendPlChangesCommand(mpd_Connection * connection, long long playlist);

/**
 * @param connection: A valid and connected mpd_Connection.
 * @param playlist: The playlist version you want the diff with.
 * A more bandwidth efficient version of the mpd_sendPlChangesCommand.
 * It only returns the pos+id of the changes song.
 */
void mpd_sendPlChangesPosIdCommand(mpd_Connection * connection, long long playlist);

/* recursivel fetches all songs/dir/playlists in "dir* (no metadata is
 * returned) */
void mpd_sendListallCommand(mpd_Connection * connection, const char * dir);

/* same as sendListallCommand, but also metadata is returned */
void mpd_sendListallInfoCommand(mpd_Connection * connection, const char * dir);

/* non-recursive version of ListallInfo */
void mpd_sendLsInfoCommand(mpd_Connection * connection, const char * dir);

#define MPD_TABLE_ARTIST	MPD_TAG_ITEM_ARTIST
#define MPD_TABLE_ALBUM		MPD_TAG_ITEM_ALBUM
#define MPD_TABLE_TITLE		MPD_TAG_ITEM_TITLE
#define MPD_TABLE_FILENAME	MPD_TAG_ITEM_FILENAME

void mpd_sendSearchCommand(mpd_Connection * connection, int table,
		const char * str);

void mpd_sendFindCommand(mpd_Connection * connection, int table,
		const char * str);

/* LIST TAG COMMANDS */

/* use this function fetch next artist entry, be sure to free the returned
 * string.  NULL means there are no more.  Best used with sendListArtists
 */
char * mpd_getNextArtist(mpd_Connection * connection);

char * mpd_getNextAlbum(mpd_Connection * connection);

char * mpd_getNextTag(mpd_Connection *connection, int type);

/* list artist or albums by artist, arg1 should be set to the artist if
 * listing albums by a artist, otherwise NULL for listing all artists or albums
 */
void mpd_sendListCommand(mpd_Connection * connection, int table,
		const char * arg1);

/* SIMPLE COMMANDS */

void mpd_sendAddCommand(mpd_Connection * connection, const char * file);

int mpd_sendAddIdCommand(mpd_Connection *connection, const char *file);

void mpd_sendDeleteCommand(mpd_Connection * connection, int songNum);

void mpd_sendDeleteIdCommand(mpd_Connection * connection, int songNum);

void mpd_sendSaveCommand(mpd_Connection * connection, const char * name);

void mpd_sendLoadCommand(mpd_Connection * connection, const char * name);

void mpd_sendRmCommand(mpd_Connection * connection, const char * name);

void mpd_sendRenameCommand(mpd_Connection *connection, const char *from,
                           const char *to);

void mpd_sendShuffleCommand(mpd_Connection * connection);

void mpd_sendClearCommand(mpd_Connection * connection);

/* use this to start playing at the beginning, useful when in random mode */
#define MPD_PLAY_AT_BEGINNING	-1

void mpd_sendPlayCommand(mpd_Connection * connection, int songNum);

void mpd_sendPlayIdCommand(mpd_Connection * connection, int songNum);

void mpd_sendStopCommand(mpd_Connection * connection);

void mpd_sendPauseCommand(mpd_Connection * connection, int pauseMode);

void mpd_sendNextCommand(mpd_Connection * connection);

void mpd_sendPrevCommand(mpd_Connection * connection);

void mpd_sendMoveCommand(mpd_Connection * connection, int from, int to);

void mpd_sendMoveIdCommand(mpd_Connection * connection, int from, int to);

void mpd_sendSwapCommand(mpd_Connection * connection, int song1, int song2);

void mpd_sendSwapIdCommand(mpd_Connection * connection, int song1, int song2);

void mpd_sendSeekCommand(mpd_Connection * connection, int song, int time);

void mpd_sendSeekIdCommand(mpd_Connection * connection, int song, int time);

void mpd_sendRepeatCommand(mpd_Connection * connection, int repeatMode);

void mpd_sendRandomCommand(mpd_Connection * connection, int randomMode);

void mpd_sendSetvolCommand(mpd_Connection * connection, int volumeChange);

/* WARNING: don't use volume command, its depreacted */
void mpd_sendVolumeCommand(mpd_Connection * connection, int volumeChange);

void mpd_sendCrossfadeCommand(mpd_Connection * connection, int seconds);

void mpd_sendUpdateCommand(mpd_Connection * connection, const char *path);

/* returns the update job id, call this after a update command*/
int mpd_getUpdateId(mpd_Connection * connection);

void mpd_sendPasswordCommand(mpd_Connection * connection, const char * pass);

/* after executing a command, when your done with it to get its status
 * (you want to check connection->error for an error)
 */
void mpd_finishCommand(mpd_Connection * connection);

/* command list stuff, use this to do things like add files very quickly */
void mpd_sendCommandListBegin(mpd_Connection * connection);

void mpd_sendCommandListOkBegin(mpd_Connection * connection);

void mpd_sendCommandListEnd(mpd_Connection * connection);

/* advance to the next listOk
 * returns 0 if advanced to the next list_OK,
 * returns -1 if it advanced to an OK or ACK */
int mpd_nextListOkCommand(mpd_Connection * connection);

typedef struct _mpd_OutputEntity {
	int id;
	char * name;
	int enabled;
} mpd_OutputEntity;

void mpd_sendOutputsCommand(mpd_Connection * connection);

mpd_OutputEntity * mpd_getNextOutput(mpd_Connection * connection);

void mpd_sendEnableOutputCommand(mpd_Connection * connection, int outputId);

void mpd_sendDisableOutputCommand(mpd_Connection * connection, int outputId);

void mpd_freeOutputElement(mpd_OutputEntity * output);

/**
 * @param connection a #mpd_Connection
 *
 * Queries mpd for the allowed commands
 */
void mpd_sendCommandsCommand(mpd_Connection * connection);

/**
 * @param connection a #mpd_Connection
 *
 * Queries mpd for the not allowed commands
 */
void mpd_sendNotCommandsCommand(mpd_Connection * connection);

/**
 * @param connection a #mpd_Connection
 *
 * returns the next supported command.
 *
 * @returns a string, needs to be free'ed
 */
char *mpd_getNextCommand(mpd_Connection *connection);

void mpd_sendUrlHandlersCommand(mpd_Connection * connection);

char *mpd_getNextHandler(mpd_Connection * connection);

void mpd_sendTagTypesCommand(mpd_Connection * connection);

char *mpd_getNextTagType(mpd_Connection * connection);

/**
 * @param connection a MpdConnection
 * @param path	the path to the playlist.
 *
 * List the content, with full metadata, of a stored playlist.
 *
 */
void mpd_sendListPlaylistInfoCommand(mpd_Connection *connection, char *path);

/**
 * @param connection a MpdConnection
 * @param path	the path to the playlist.
 *
 * List the content of a stored playlist.
 *
 */
void mpd_sendListPlaylistCommand(mpd_Connection *connection, char *path);

/**
 * @param connection a #mpd_Connection
 * @param exact if to match exact
 *
 * starts a search, use mpd_addConstraintSearch to add
 * a constraint to the search, and mpd_commitSearch to do the actual search
 */
void mpd_startSearch(mpd_Connection *connection, int exact);

/**
 * @param connection a #mpd_Connection
 * @param type
 * @param name
 */
void mpd_addConstraintSearch(mpd_Connection *connection, int type, const char *name);

/**
 * @param connection a #mpd_Connection
 */
void mpd_commitSearch(mpd_Connection *connection);

/**
 * @param connection a #mpd_Connection
 * @param type The type to search for
 *
 * starts a search for fields... f.e. get a list of artists would be:
 * @code
 * mpd_startFieldSearch(connection, MPD_TAG_ITEM_ARTIST);
 * mpd_commitSearch(connection);
 * @endcode
 *
 * or get a list of artist in genre "jazz" would be:
 * @code
 * mpd_startFieldSearch(connection, MPD_TAG_ITEM_ARTIST);
 * mpd_addConstraintSearch(connection, MPD_TAG_ITEM_GENRE, "jazz")
 * mpd_commitSearch(connection);
 * @endcode
 *
 * mpd_startSearch will return  a list of songs (and you need mpd_getNextInfoEntity)
 * this one will return a list of only one field (the one specified with type) and you need
 * mpd_getNextTag to get the results
 */
void mpd_startFieldSearch(mpd_Connection *connection, int type);

void mpd_startPlaylistSearch(mpd_Connection *connection, int exact);

void mpd_startStatsSearch(mpd_Connection *connection);

void mpd_sendPlaylistClearCommand(mpd_Connection *connection, char *path);

void mpd_sendPlaylistAddCommand(mpd_Connection *connection,
                                char *playlist, char *path);

void mpd_sendPlaylistMoveCommand(mpd_Connection *connection,
                                 char *playlist, int from, int to);

void mpd_sendPlaylistDeleteCommand(mpd_Connection *connection,
                                   char *playlist, int pos);

void mpd_startIdle(mpd_Connection *connection, mpd_NotificationCb notify_cb, void *userdata);

void mpd_stopIdle(mpd_Connection *connection);

#ifdef MPD_GLIB
void mpd_glibInit(mpd_Connection *connection);
#endif

#ifdef __cplusplus
}
#endif

#endif
