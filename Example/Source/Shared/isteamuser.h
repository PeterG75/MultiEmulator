#pragma once

struct CSteamID;
struct CGameID;

class VSteamUser
{
	// returns the HSteamUser this interface represents
	// this is only used internally by the API, and by a few select interfaces that support multi-user
	virtual int GetHSteamUser() abstract;

	// returns true if the Steam client current has a live connection to the Steam servers. 
	// If false, it means there is no active connection due to either a networking issue on the local machine, or the Steam server is down/busy.
	// The Steam client will automatically be trying to recreate the connection as often as possible.
	virtual bool BLoggedOn() abstract;

	// returns the CSteamID of the account currently logged into the Steam client
	// a CSteamID is a unique identifier for an account, and used to differentiate users in all parts of the Steamworks API
	virtual CSteamID GetSteamID() abstract;

	// Multiplayer Authentication functions
	
	// InitiateGameConnection() starts the state machine for authenticating the game client with the game server
	// It is the client portion of a three-way handshake between the client, the game server, and the steam servers
	//
	// Parameters:
	// void *pAuthBlob - a pointer to empty memory that will be filled in with the authentication token.
	// int cbMaxAuthBlob - the number of bytes of allocated memory in pBlob. Should be at least 2048 bytes.
	// CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
	// CGameID gameID - the ID of the current game. For games without mods, this is just CGameID( <appID> )
	// unsigned int unIPServer, unsigned short usPortServer - the IP address of the game server
	// bool bSecure - whether or not the client thinks that the game server is reporting itself as secure (i.e. VAC is running)
	//
	// return value - returns the number of bytes written to pBlob. If the return is 0, then the buffer passed in was too small, and the call has failed
	// The contents of pBlob should then be sent to the game server, for it to use to complete the authentication process.
	virtual int InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, unsigned int unIPServer, unsigned short usPortServer, bool bSecure ) abstract;

	// notify of disconnect
	// needs to occur when the game client leaves the specified game server, needs to match with the InitiateGameConnection() call
	virtual void TerminateGameConnection( unsigned int unIPServer, unsigned short usPortServer ) abstract;

	// Legacy functions

	// used by only a few games to track usage events
	virtual void TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo = "" ) abstract;

	// get the local storage folder for current Steam account to write application data, e.g. save games, configs etc.
	// this will usually be something like "C:\Progam Files\Steam\userdata\<SteamID>\<AppID>\local"
	virtual bool GetUserDataFolder( char *pchBuffer, int cubBuffer ) abstract;

	// Starts voice recording. Once started, use GetVoice() to get the data
	virtual void StartVoiceRecording( ) abstract;

	// Stops voice recording. Because people often release push-to-talk keys early, the system will keep recording for
	// a little bit after this function is called. GetVoice() should continue to be called until it returns
	// k_eVoiceResultNotRecording
	virtual void StopVoiceRecording( ) abstract;

	// Determine the amount of captured audio data that is available in bytes.
	// This provides both the compressed and uncompressed data. Please note that the uncompressed
	// data is not the raw feed from the microphone: data may only be available if audible 
	// levels of speech are detected.
	// nUncompressedVoiceDesiredSampleRate is necessary to know the number of bytes to return in pcbUncompressed - can be set to 0 if you don't need uncompressed (the usual case)
	// If you're upgrading from an older Steamworks API, you'll want to pass in 11025 to nUncompressedVoiceDesiredSampleRate
	virtual int GetAvailableVoice( unsigned int *pcbCompressed, unsigned int *pcbUncompressed, unsigned int nUncompressedVoiceDesiredSampleRate ) abstract;

	// Gets the latest voice data from the microphone. Compressed data is an arbitrary format, and is meant to be handed back to 
	// DecompressVoice() for playback later as a binary blob. Uncompressed data is 16-bit, signed integer, 11025Hz PCM format.
	// Please note that the uncompressed data is not the raw feed from the microphone: data may only be available if audible 
	// levels of speech are detected, and may have passed through denoising filters, etc.
	// This function should be called as often as possible once recording has started; once per frame at least.
	// nBytesWritten is set to the number of bytes written to pDestBuffer. 
	// nUncompressedBytesWritten is set to the number of bytes written to pUncompressedDestBuffer. 
	// You must grab both compressed and uncompressed here at the same time, if you want both.
	// Matching data that is not read during this call will be thrown away.
	// GetAvailableVoice() can be used to determine how much data is actually available.
	// If you're upgrading from an older Steamworks API, you'll want to pass in 11025 to nUncompressedVoiceDesiredSampleRate
	virtual int GetVoice( bool bWantCompressed, void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten, bool bWantUncompressed, void *pUncompressedDestBuffer, unsigned int cbUncompressedDestBufferSize, unsigned int *nUncompressBytesWritten, unsigned int nUncompressedVoiceDesiredSampleRate ) abstract;

	// Decompresses a chunk of compressed data produced by GetVoice().
	// nBytesWritten is set to the number of bytes written to pDestBuffer unless the return value is k_intBufferTooSmall.
	// In that case, nBytesWritten is set to the size of the buffer required to decompress the given
	// data. The suggested buffer size for the destination buffer is 22 kilobytes.
	// The output format of the data is 16-bit signed at the requested samples per second.
	// If you're upgrading from an older Steamworks API, you'll want to pass in 11025 to nDesiredSampleRate
	virtual int DecompressVoice( const void *pCompressed, unsigned int cbCompressed, void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten, unsigned int nDesiredSampleRate ) abstract;

	// This returns the frequency of the voice data as it's stored internally; calling DecompressVoice() with this size will yield the best results
	virtual unsigned int GetVoiceOptimalSampleRate() abstract;

	// Retrieve ticket to be sent to the entity who wishes to authenticate you. 
	// pcbTicket retrieves the length of the actual ticket.
	virtual int GetAuthSessionTicket( void *pTicket, int cbMaxTicket, unsigned int *pcbTicket ) abstract;

	// Authenticate ticket from entity steamID to be sure it is valid and isnt reused
	// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
	virtual int BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID ) abstract;

	// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
	virtual void EndAuthSession( CSteamID steamID ) abstract;

	// Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
	virtual void CancelAuthTicket( int hAuthTicket ) abstract;

	// After receiving a user's authentication data, and passing it to BeginAuthSession, use this function
	// to determine if the user owns downloadable content specified by the provided AppID.
	virtual int UserHasLicenseForApp( CSteamID steamID, int appID ) abstract;
	
	// returns true if this users looks like they are behind a NAT device. Only valid once the user has connected to steam 
	// (i.e a SteamServersConnected_t has been issued) and may not catch all forms of NAT.
	virtual bool BIsBehindNAT() abstract;

	// set data to be replicated to friends so that they can join your game
	// CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
	// unsigned int unIPServer, unsigned short usPortServer - the IP address of the game server
	virtual void AdvertiseGame( CSteamID steamIDGameServer, unsigned int unIPServer, unsigned short usPortServer ) abstract;

	// Requests a ticket encrypted with an app specific shared key
	// pDataToInclude, cbDataToInclude will be encrypted into the ticket
	// ( This is asynchronous, you must wait for the ticket to be completed by the server )
	virtual long long RequestEncryptedAppTicket( void *pDataToInclude, int cbDataToInclude ) abstract;

	// retrieve a finished ticket
	virtual bool GetEncryptedAppTicket( void *pTicket, int cbMaxTicket, unsigned int *pcbTicket ) abstract;

	// Trading Card badges data access
	// if you only have one set of cards, the series will be 1
	// the user has can have two different badges for a series; the regular (max level 5) and the foil (max level 1)
	virtual int GetGameBadgeLevel( int nSeries, bool bFoil ) abstract;

	// gets the Steam Level of the user, as shown on their profile
	virtual int GetPlayerSteamLevel() abstract;

	// Requests a URL which authenticates an in-game browser for store check-out,
	// and then redirects to the specified URL. As long as the in-game browser
	// accepts and handles session cookies, Steam microtransaction checkout pages
	// will automatically recognize the user instead of presenting a login page.
	// The result of this API call will be a StoreAuthURLResponse_t callback.
	// NOTE: The URL has a very short lifetime to prevent history-snooping attacks,
	// so you should only call this API when you are about to launch the browser,
	// or else immediately navigate to the result URL using a hidden browser window.
	// NOTE 2: The resulting authorization cookie has an expiration time of one day,
	// so it would be a good idea to request and visit a new auth URL every 12 hours.
	virtual long long RequestStoreAuthURL( const char *pchRedirectURL ) abstract;
};

struct ISteamUser
{
	VSteamUser* _vptr;
};