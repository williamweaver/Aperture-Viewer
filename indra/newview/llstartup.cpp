/** 
 * @file llstartup.cpp
 * @brief startup routines.
 *
 * $LicenseInfo:firstyear=2004&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "llappviewer.h"
#include "llstartup.h"
#include "llcallstack.h"

#if LL_WINDOWS
#	include <process.h>		// _spawnl()
#else
#	include <sys/stat.h>		// mkdir()
#endif

#include "llviewermedia_streamingaudio.h"
#include "llaudioengine.h"

#ifdef LL_FMODSTUDIO
# include "llaudioengine_fmodstudio.h"
#endif

#ifdef LL_OPENAL
#include "llaudioengine_openal.h"
#endif

#include "llavatarnamecache.h"
#include "llexperiencecache.h"
#include "lllandmark.h"
#include "llcachename.h"
#include "lldir.h"
#include "lldonotdisturbnotificationstorage.h"
#include "llerrorcontrol.h"
#include "llfloaterreg.h"
#include "llfocusmgr.h"
#include "llfloatergridstatus.h"
#include "llfloaterimsession.h"
#include "lllocationhistory.h"
#include "llimageworker.h"

#include "llloginflags.h"
#include "llmd5.h"
#include "llmemorystream.h"
#include "llmessageconfig.h"
#include "llmoveview.h"
#include "llfloaterimcontainer.h"
#include "llfloaterimnearbychat.h"
#include "llnotifications.h"
#include "llnotificationsutil.h"
#include "llpersistentnotificationstorage.h"
#include "llpresetsmanager.h"
#include "llteleporthistory.h"
#include "llregionhandle.h"
#include "llsd.h"
#include "llsdserialize.h"
#include "llsdutil_math.h"
#include "llstring.h"
#include "lluserrelations.h"
#include "llversioninfo.h"
#include "llviewercontrol.h"
#include "llviewerhelp.h"
#include "llxorcipher.h"	// saved password, MAC address
#include "llwindow.h"
#include "message.h"
#include "v3math.h"

#include "llagent.h"
#include "llagentbenefits.h"
#include "llagentcamera.h"
#include "llagentpicksinfo.h"
#include "llagentwearables.h"
#include "llagentpilot.h"
#include "llfloateravatarpicker.h"
#include "llcallbacklist.h"
#include "llcallingcard.h"
#include "llconsole.h"
#include "llcontainerview.h"
#include "llconversationlog.h"
#include "lldebugview.h"
#include "lldrawable.h"
#include "lleventnotifier.h"
#include "llface.h"
#include "llfeaturemanager.h"
//#include "llfirstuse.h"
#include "llfloaterhud.h"
#include "llfloaterland.h"
#include "llfloatertopobjects.h"
#include "llfloaterworldmap.h"
#include "llgesturemgr.h"
#include "llgroupmgr.h"
#include "llhudeffecttrail.h"
#include "llhudmanager.h"
#include "llbufferstream.h" // <FS:PP> For SL Grid Status
#include "llimage.h"
#include "llinventorybridge.h"
#include "llinventorymodel.h"
#include "llinventorymodelbackgroundfetch.h"
#include "llkeyboard.h"
#include "llloginhandler.h"			// gLoginHandler, SLURL support
#include "lllogininstance.h" // Host the login module.
// <FS:Ansariel> [FS Login Panel]
//#include "llpanellogin.h"
#include "fspanellogin.h"
// <FS:Ansariel> [FS Login Panel]
#include "llmutelist.h"
#include "llavatarpropertiesprocessor.h"
#include "llpanelclassified.h"
#include "llpanelpick.h"
#include "llpanelgrouplandmoney.h"
#include "llpanelgroupnotices.h"
#include "llparcel.h"
#include "llpreview.h"
#include "llpreviewscript.h"
#include "llproxy.h"
#include "llproductinforequest.h"
#include "llqueryflags.h"
#include "llselectmgr.h"
#include "llsky.h"
#include "llstatview.h"
#include "llstatusbar.h"		// sendMoneyBalanceRequest(), owns L$ balance
#include "llsurface.h"
#include "lltexturecache.h"
#include "lltexturefetch.h"
#include "lltoolmgr.h"
#include "lltrans.h"
#include "llui.h"
#include "lluiusage.h"
#include "llurldispatcher.h"
#include "llurlentry.h"
#include "llslurl.h"
#include "llurlhistory.h"
#include "llurlwhitelist.h"
#include "llvieweraudio.h"
#include "llviewerassetstorage.h"
#include "llviewercamera.h"
#include "llviewerdisplay.h"
#include "llviewergenericmessage.h"
#include "llviewergesture.h"
#include "llviewertexturelist.h"
#include "llviewermedia.h"
#include "llviewermenu.h"
#include "llviewermessage.h"
#include "llviewernetwork.h"
#include "llviewerobjectlist.h"
#include "llviewerparcelaskplay.h"
#include "llviewerparcelmedia.h"
#include "llviewerparcelmgr.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llviewerstatsrecorder.h"
#include "llviewerthrottle.h"
#include "llviewerwindow.h"
#include "llvoavatar.h"
#include "llvoavatarself.h"
#include "llweb.h"
#include "llworld.h"
#include "llworldmapmessage.h"
#include "llxfermanager.h"
#include "pipeline.h"
#include "llappviewer.h"
#include "llfasttimerview.h"
#include "llfloatermap.h"
#include "llweb.h"
#include "llvoiceclient.h"
#include "llnamelistctrl.h"
#include "llnamebox.h"
#include "llnameeditor.h"
#include "llpostprocess.h"
#include "llagentlanguage.h"
#include "llwearable.h"
#include "llinventorybridge.h"
#include "llappearancemgr.h"
#include "llavatariconctrl.h"
#include "llvoicechannel.h"
#include "llpathfindingmanager.h"
// [RLVa:KB] - Checked: RLVa-1.2.0
#include "rlvhandler.h"
// [/RLVa:KB]

#include "lllogin.h"
#include "llevents.h"
#include "llstartuplistener.h"
#include "lltoolbarview.h"
#include "llexperiencelog.h"
#include "llcleanup.h"

#include "llenvironment.h"

#include "llstacktrace.h"

#if LL_WINDOWS
#include "lldxhardware.h"
#endif

// Firestorm includes
// <FS:Ansariel> [FS communication UI]
//#include "llfloaterimsession.h"
#include "fsfloaterim.h"
// </FS:Ansariel> [FS communication UI]
#if HAS_GROWL
#include "growlmanager.h"
#endif

#include "fsassetblacklist.h"
#include "fsavatarrenderpersistence.h"
#include "fscommon.h"
#include "fscorehttputil.h"
#include "fsdata.h"
#include "fsfloatercontacts.h"
#include "fsfloaterimcontainer.h"
#include "fsfloaternearbychat.h"
#include "fsfloatersearch.h"
#include "fsfloaterwearablefavorites.h"
#include "fslslbridge.h"
#include "fsradar.h"
#include "fsregistrarutils.h"
#include "fsscriptlibrary.h"
#include "lfsimfeaturehandler.h"
#include "lggcontactsets.h"
#include "llfloatersearch.h"
#include "llfloatersidepanelcontainer.h"
#include "llfriendcard.h"
#include "llnotificationmanager.h"
#include "llpresetsmanager.h"
#include "llprogressview.h"
#include "lltoolbarview.h"
#include "NACLantispam.h"
#include "streamtitledisplay.h"
#include "tea.h"

//
// exported globals
//
bool gAgentMovementCompleted = false;
S32  gMaxAgentGroups; // <FS:Ansariel> OpenSim legacy economy support

const std::string SCREEN_HOME_FILENAME = "screen_home%s.png";
const std::string SCREEN_LAST_FILENAME = "screen_last%s.png";

LLPointer<LLViewerTexture> gStartTexture;

//
// Imported globals
//
extern S32 gStartImageWidth;
extern S32 gStartImageHeight;
extern std::string gWindowTitle;

//
// local globals
//
static bool gGotUseCircuitCodeAck = false;
static std::string sInitialOutfit;
static std::string sInitialOutfitGender;	// "male" or "female"

static bool gUseCircuitCallbackCalled = false;

EStartupState LLStartUp::gStartupState = STATE_FIRST;
LLSLURL LLStartUp::sStartSLURL;
std::string LLStartUp::sStartSLURLString;

static LLPointer<LLCredential> gUserCredential;
static std::string gDisplayName;
static bool gRememberPassword = true;
static bool gRememberUser = true;

static U64 gFirstSimHandle = 0;
static LLHost gFirstSim;
static std::string gFirstSimSeedCap;
static LLVector3 gAgentStartLookAt(1.0f, 0.f, 0.f);
static std::string gAgentStartLocation = "safe";
static bool mLoginStatePastUI = false;
static bool mBenefitsSuccessfullyInit = false;

const F32 STATE_AGENT_WAIT_TIMEOUT = 240; //seconds

boost::scoped_ptr<LLEventPump> LLStartUp::sStateWatcher(new LLEventStream("StartupState"));
boost::scoped_ptr<LLStartupListener> LLStartUp::sListener(new LLStartupListener());
boost::scoped_ptr<LLViewerStats::PhaseMap> LLStartUp::sPhases(new LLViewerStats::PhaseMap);

//
// local function declaration
//

void login_show();
void login_callback(S32 option, void* userdata);
void show_release_notes_if_required();
//void show_first_run_dialog();	// <FS:CR> Unused in Firestorm
bool first_run_dialog_callback(const LLSD& notification, const LLSD& response);
void set_startup_status(const F32 frac, const std::string& string, const std::string& msg);
bool login_alert_status(const LLSD& notification, const LLSD& response);
void login_packet_failed(void**, S32 result);
void use_circuit_callback(void**, S32 result);
void register_viewer_callbacks(LLMessageSystem* msg);
void asset_callback_nothing(const LLUUID&, LLAssetType::EType, void*, S32);
bool callback_choose_gender(const LLSD& notification, const LLSD& response);
void init_start_screen(S32 location_id);
void release_start_screen();
void reset_login();
LLSD transform_cert_args(LLPointer<LLCertificate> cert);
void general_cert_done(const LLSD& notification, const LLSD& response);
void trust_cert_done(const LLSD& notification, const LLSD& response);
void apply_udp_blacklist(const std::string& csv);
// <FS:CR> Aurora Sim
//bool process_login_success_response();
bool process_login_success_response(U32 &first_sim_size_x, U32 &first_sim_size_y);
// </FS:CR> Aurora Sim
void on_benefits_failed_callback(const LLSD& notification, const LLSD& response);
void transition_back_to_login_panel(const std::string& emsg);
// <FS:KC> FIRE-18250: Option to disable default eye movement
void update_static_eyes();
// </FS:KC> FIRE-18250

void callback_cache_name(const LLUUID& id, const std::string& full_name, bool is_group)
{
	LLNameBox::refreshAll(id, full_name, is_group);
	LLNameEditor::refreshAll(id, full_name, is_group);
	
	// TODO: Actually be intelligent about the refresh.
	// For now, just brute force refresh the dialogs.
	dialog_refresh_all();
}

//
// exported functionality
//

//
// local classes
//

// <AW: opensim>
static bool sGridListRequestReady = false;
void downloadGridlistComplete( LLSD const &aData )
{
	LL_DEBUGS() << aData << LL_ENDL;
	
	LLSD header = aData[ LLCoreHttpUtil::HttpCoroutineAdapter::HTTP_RESULTS ][ LLCoreHttpUtil::HttpCoroutineAdapter::HTTP_RESULTS_HEADERS];

	LLDate lastModified;
	if (header.has("last-modified"))
	{
		lastModified.secondsSinceEpoch( FSCommon::secondsSinceEpochFromString( "%a, %d %b %Y %H:%M:%S %ZP", header["last-modified"].asString() ) );
	}

	LLSD data = aData;
	data.erase( LLCoreHttpUtil::HttpCoroutineAdapter::HTTP_RESULTS );
	
	std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "grids.remote.xml");

	llofstream out_file;
	out_file.open(filename.c_str());
	LLSDSerialize::toPrettyXML( data, out_file);
	out_file.close();
	LL_INFOS() << "GridListRequest: got new list." << LL_ENDL;
	sGridListRequestReady = true;
}

void downloadGridlistError( LLSD const &aData, std::string const &aURL )
{
	LLCore::HttpStatus status = LLCoreHttpUtil::HttpCoroutineAdapter::getStatusFromLLSD(aData);

	if (status.getType() == HTTP_NOT_MODIFIED)
	{
		LL_INFOS("fsdata") << "Didn't download grid list from " << aURL << " - no newer version available" << LL_ENDL;
	}
	else
	{
		LL_WARNS() << "Failed to download grid list from " << aURL << LL_ENDL;
	}

	sGridListRequestReady = true;
}

 void downloadGridstatusComplete(LLSD const &aData)
{
	LLSD header = aData[ LLCoreHttpUtil::HttpCoroutineAdapter::HTTP_RESULTS ][ LLCoreHttpUtil::HttpCoroutineAdapter::HTTP_RESULTS_HEADERS];
	const LLSD::Binary &rawData = aData[LLCoreHttpUtil::HttpCoroutineAdapter::HTTP_RESULTS_RAW].asBinary();

	if (rawData.size() == 0)
	{
		report_to_nearby_chat(LLTrans::getString("SLGridStatusInvalidMsg"));
		LL_WARNS("SLGridStatusResponder") << "Error - empty output" << LL_ENDL;
		return;
	}

	std::string fetchedNews;
	fetchedNews.assign( rawData.begin(), rawData.end() );

	size_t itemStart = fetchedNews.find("<item>");
	size_t itemEnd = fetchedNews.find("</item>");
	if (itemEnd != std::string::npos && itemStart != std::string::npos)
	{
		// Isolate latest news data
		itemStart += 6;
		std::string theNews = fetchedNews.substr(itemStart, itemEnd - itemStart);

		// Check for and remove CDATA characters if they're present
		size_t titleStart = theNews.find("<title><![CDATA[");
		if (titleStart != std::string::npos)
		{
			theNews.replace(titleStart, 16, "<title>");
		}
		size_t titleEnd = theNews.find("]]></title>");
		if (titleEnd != std::string::npos)
		{
			theNews.replace(titleEnd, 11, "</title>");
		}
		size_t descStart = theNews.find("<description><![CDATA[");
		if (descStart != std::string::npos)
		{
			theNews.replace(descStart, 22, "<description>");
		}
		size_t descEnd = theNews.find("]]></description>");
		if (descEnd != std::string::npos)
		{
			theNews.replace(descEnd, 17, "</description>");
		}
		size_t linkStart = theNews.find("<link><![CDATA[");
		if (linkStart != std::string::npos)
		{
			theNews.replace(linkStart, 15, "<link>");
		}
		size_t linkEnd = theNews.find("]]></link>");
		if (linkEnd != std::string::npos)
		{
			theNews.replace(linkEnd, 10, "</link>");
		}

		// Get indexes
		titleStart = theNews.find("<title>");
		descStart = theNews.find("<description>");
		linkStart = theNews.find("<link>");
		titleEnd = theNews.find("</title>");
		descEnd = theNews.find("</description>");
		linkEnd = theNews.find("</link>");

		if (titleStart != std::string::npos &&
			descStart != std::string::npos &&
			linkStart != std::string::npos &&
			titleEnd != std::string::npos &&
			descEnd != std::string::npos &&
			linkEnd != std::string::npos)
		{
			titleStart += 7;
			descStart += 13;
			linkStart += 6;
			std::string newsTitle = theNews.substr(titleStart, titleEnd - titleStart);
			std::string newsDesc = theNews.substr(descStart, descEnd - descStart);
			std::string newsLink = theNews.substr(linkStart, linkEnd - linkStart);
			LLStringUtil::trim(newsTitle);
			LLStringUtil::trim(newsDesc);
			LLStringUtil::trim(newsLink);
			report_to_nearby_chat("[ " + newsTitle + " ] " + newsDesc + " [ " + newsLink + " ]");
		}
		else
		{
			report_to_nearby_chat(LLTrans::getString("SLGridStatusInvalidMsg"));
			LL_WARNS("SLGridStatusResponder") << "Error - inner tag(s) missing" << LL_ENDL;
		}
	}
	else
	{
		report_to_nearby_chat(LLTrans::getString("SLGridStatusInvalidMsg"));
		LL_WARNS("SLGridStatusResponder") << "Error - output without </item>" << LL_ENDL;
	}
}

void downloadGridstatusError(LLSD const &aData, std::string const &aURL)
{
	LLCore::HttpStatus status = LLCoreHttpUtil::HttpCoroutineAdapter::getStatusFromLLSD(aData);
	LL_WARNS("SLGridStatusResponder") << "Error - status " << status.getType() << LL_ENDL;

	if (status.getType() == HTTP_INTERNAL_ERROR)
	{
		report_to_nearby_chat(LLTrans::getString("SLGridStatusTimedOut"));
	}
	else
	{
		LLStringUtil::format_map_t args;
		args["STATUS"] = llformat("%d", status.getType());
		report_to_nearby_chat(LLTrans::getString("SLGridStatusOtherError", args));
	}
}
// </FS:PP>

// <FS:Ansariel> Check for test build expiration
bool is_testbuild_expired()
{
#if TESTBUILD
	std::string datestr = __DATE__;

	std::istringstream iss_date(datestr);
	std::string str_month;
	S32 day;
	S32 year;
	S32 month = 1;
	iss_date >> str_month >> day >> year;

	if (str_month == "Jan") month = 1;
	else if (str_month == "Feb") month = 2;
	else if (str_month == "Mar") month = 3;
	else if (str_month == "Apr") month = 4;
	else if (str_month == "May") month = 5;
	else if (str_month == "Jun") month = 6;
	else if (str_month == "Jul") month = 7;
	else if (str_month == "Aug") month = 8;
	else if (str_month == "Sep") month = 9;
	else if (str_month == "Oct") month = 10;
	else if (str_month == "Nov") month = 11;
	else if (str_month == "Dec") month = 12;

	tm t = {0};
	t.tm_mon = month - 1;
	t.tm_mday = day;
	t.tm_year = year - 1900;
	t.tm_hour = 0;
	t.tm_min = 0;
	t.tm_sec = 0;

	time_t expiry_time = mktime(&t) + (S32(TESTBUILDPERIOD) + 1) * 24 * 60 * 60;
	time_t current_time = time(NULL);

	return current_time > expiry_time;
#else
	return false;
#endif
}
// </FS:Ansariel>

void update_texture_fetch()
{
	LLAppViewer::getTextureCache()->update(1); // unpauses the texture cache thread
	LLAppViewer::getImageDecodeThread()->update(1); // unpauses the image thread
	LLAppViewer::getTextureFetch()->update(1); // unpauses the texture fetch thread
	gTextureList.updateImages(0.10f);
}

void set_flags_and_update_appearance()
{
	LLAppearanceMgr::instance().setAttachmentInvLinkEnable(true);
	LLAppearanceMgr::instance().updateAppearanceFromCOF(true, true, no_op);
}

// Returns false to skip other idle processing. Should only return
// true when all initialization done.
bool idle_startup()
{
	if (gViewerWindow == NULL)
	{
		// We expect window to be initialized
		LL_WARNS_ONCE() << "gViewerWindow is not initialized" << LL_ENDL;
		return false; // No world yet
	}

	const F32 PRECACHING_DELAY = gSavedSettings.getF32("PrecachingDelay");
	static LLTimer timeout;

	static LLTimer login_time;

	// until this is encapsulated, this little hack for the
	// auth/transform loop will do.
	static F32 progress = 0.10f;

	static std::string auth_desc;
	static std::string auth_message;

// <FS:CR> Aurora Sim
	static U32 first_sim_size_x = 256;
	static U32 first_sim_size_y = 256;
// </FS:CR> Aurora Sim
	static LLVector3 agent_start_position_region(10.f, 10.f, 10.f);		// default for when no space server

	// last location by default
	static S32  agent_location_id = START_LOCATION_ID_LAST;

	static bool show_connect_box = true;

	//static bool stipend_since_login = false;

	// HACK: These are things from the main loop that usually aren't done
	// until initialization is complete, but need to be done here for things
	// to work.
	gIdleCallbacks.callFunctions();
	gViewerWindow->updateUI();

	LLMortician::updateClass();

	const std::string delims (" ");
	std::string system;
	int begIdx, endIdx;
	std::string osString = LLOSInfo::instance().getOSStringSimple();

	begIdx = osString.find_first_not_of (delims);
	endIdx = osString.find_first_of (delims, begIdx);
	system = osString.substr (begIdx, endIdx - begIdx);
	system += "Locale";

	LLStringUtil::setLocale (LLTrans::getString(system));

	//note: Removing this line will cause incorrect button size in the login screen. -- bao.
	gTextureList.updateImages(0.01f) ;

	if ( STATE_FIRST == LLStartUp::getStartupState() )
	{
		static bool first_call = true;
		if (first_call)
		{
			// Other phases get handled when startup state changes,
			// need to capture the initial state as well.
			LLStartUp::getPhases().startPhase(LLStartUp::getStartupStateString());
			first_call = false;
		}

		gViewerWindow->showCursor(); 
		gViewerWindow->getWindow()->setCursor(UI_CURSOR_WAIT);

		std::string beamsFolder(gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "beams",""));
		LLFile::mkdir(beamsFolder.c_str());
		
		std::string beamsColorsFolder(gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "beamsColors",""));
		LLFile::mkdir(beamsColorsFolder.c_str());


		/////////////////////////////////////////////////
		//
		// Initialize stuff that doesn't need data from simulators
		//
		std::string lastGPU = gSavedSettings.getString("LastGPUString");
		std::string thisGPU = LLFeatureManager::getInstance()->getGPUString();
		
#if HAS_GROWL
		GrowlManager::initiateManager();
#endif

		// <FS:Ansariel> Store current font and skin for system info (FIRE-6806)
		gSavedSettings.setString("FSInternalFontSettingsFile", gSavedSettings.getString("FSFontSettingsFile"));
		gSavedSettings.setString("FSInternalSkinCurrent", gSavedSettings.getString("FSSkinCurrentReadableName"));
		gSavedSettings.setString("FSInternalSkinCurrentTheme", gSavedSettings.getString("FSSkinCurrentThemeReadableName"));
		// </FS:Ansariel>

		// <FS:Ansariel> Notification not showing if hiding the UI
		gSavedSettings.setBOOL("FSInternalShowNavbarNavigationPanel", gSavedSettings.getBOOL("ShowNavbarNavigationPanel"));
		gSavedSettings.setBOOL("FSInternalShowNavbarFavoritesPanel", gSavedSettings.getBOOL("ShowNavbarFavoritesPanel"));
		// </FS:Ansariel>

		if (LLFeatureManager::getInstance()->isSafe())
		{
			LLNotificationsUtil::add("DisplaySetToSafe");
		}
		else if ((gSavedSettings.getS32("LastFeatureVersion") < LLFeatureManager::getInstance()->getVersion()) &&
				 (gSavedSettings.getS32("LastFeatureVersion") != 0))
		{
			LLNotificationsUtil::add("DisplaySetToRecommendedFeatureChange");
		}
		else if ( ! lastGPU.empty() && (lastGPU != thisGPU))
		{
			LLSD subs;
			subs["LAST_GPU"] = lastGPU;
			subs["THIS_GPU"] = thisGPU;
			LLNotificationsUtil::add("DisplaySetToRecommendedGPUChange", subs);
		}
		else if (!gViewerWindow->getInitAlert().empty())
		{
			LLNotificationsUtil::add(gViewerWindow->getInitAlert());
		}
			
		//-------------------------------------------------
		// Init the SOCKS 5 proxy if the user has configured
		// one. We need to do this early in case the user
		// is using SOCKS for HTTP so we get the login
		// screen and HTTP tables via SOCKS.
		//-------------------------------------------------
		LLStartUp::startLLProxy();

		gSavedSettings.setS32("LastFeatureVersion", LLFeatureManager::getInstance()->getVersion());
		gSavedSettings.setString("LastGPUString", thisGPU);

		std::string xml_file = LLUI::locateSkin("xui_version.xml");
		LLXMLNodePtr root;
		bool xml_ok = false;
		if (LLXMLNode::parseFile(xml_file, root, NULL))
		{
			if( (root->hasName("xui_version") ) )
			{
				std::string value = root->getValue();
				F32 version = 0.0f;
				LLStringUtil::convertToF32(value, version);
				if (version >= 1.0f)
				{
					xml_ok = true;
				}
			}
		}
		if (!xml_ok)
		{
			// If XML is bad, there's a good possibility that notifications.xml is ALSO bad.
			// If that's so, then we'll get a fatal error on attempting to load it, 
			// which will display a nontranslatable error message that says so.
			// Otherwise, we'll display a reasonable error message that IS translatable.
			LLAppViewer::instance()->earlyExit("BadInstallation");
		}
		//
		// Statistics stuff
		//

		// Load autopilot and stats stuff
		gAgentPilot.load();

		//gErrorStream.setTime(gSavedSettings.getBOOL("LogTimestamps"));

		// Load the throttle settings
		gViewerThrottle.load();

		//
		// Initialize messaging system
		//
		LL_DEBUGS("AppInit") << "Initializing messaging system..." << LL_ENDL;

		std::string message_template_path = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS,"message_template.msg");

		LLFILE* found_template = NULL;
		found_template = LLFile::fopen(message_template_path, "r");		/* Flawfinder: ignore */
		
		#if LL_WINDOWS
			// On the windows dev builds, unpackaged, the message_template.msg 
			// file will be located in:
			// build-vc**/newview/<config>/app_settings
			if (!found_template)
			{
				message_template_path = gDirUtilp->getExpandedFilename(LL_PATH_EXECUTABLE, "app_settings", "message_template.msg");
				found_template = LLFile::fopen(message_template_path.c_str(), "r");		/* Flawfinder: ignore */
			}	
		#elif LL_DARWIN
			// On Mac dev builds, message_template.msg lives in:
			// indra/build-*/newview/<config>/Second Life/Contents/Resources/app_settings
			if (!found_template)
			{
				message_template_path =
					gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS,
												   "message_template.msg");
				found_template = LLFile::fopen(message_template_path.c_str(), "r");		/* Flawfinder: ignore */
			}		
		#endif

		if (found_template)
		{
			fclose(found_template);

			U32 port = gSavedSettings.getU32("UserConnectionPort");

			if ((NET_USE_OS_ASSIGNED_PORT == port) &&   // if nothing specified on command line (-port)
			    (gSavedSettings.getBOOL("ConnectionPortEnabled")))
			  {
			    port = gSavedSettings.getU32("ConnectionPort");
			  }

			// TODO parameterize 
			const F32 circuit_heartbeat_interval = 5;
			const F32 circuit_timeout = 100;

			const LLUseCircuitCodeResponder* responder = NULL;
			bool failure_is_fatal = true;
			
			if(!start_messaging_system(
				   message_template_path,
				   port,
				   LLVersionInfo::instance().getMajor(),
				   LLVersionInfo::instance().getMinor(),
				   LLVersionInfo::instance().getPatch(),
				   FALSE,
				   std::string(),
				   responder,
				   failure_is_fatal,
				   circuit_heartbeat_interval,
				   circuit_timeout))
			{
				std::string diagnostic = llformat(" Error: %d", gMessageSystem->getErrorCode());
				LL_WARNS("AppInit") << diagnostic << LL_ENDL;
				LLAppViewer::instance()->earlyExit("LoginFailedNoNetwork", LLSD().with("DIAGNOSTIC", diagnostic));
			}

			#if LL_WINDOWS
				// On the windows dev builds, unpackaged, the message.xml file will 
				// be located in indra/build-vc**/newview/<config>/app_settings.
				std::string message_path = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS,"message.xml");
							
				if (!LLFile::isfile(message_path.c_str())) 
				{
					LLMessageConfig::initClass("viewer", gDirUtilp->getExpandedFilename(LL_PATH_EXECUTABLE, "app_settings", ""));
				}
				else
				{
					LLMessageConfig::initClass("viewer", gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, ""));
				}
			#else			
				LLMessageConfig::initClass("viewer", gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, ""));
			#endif

		}
		else
		{
			LLAppViewer::instance()->earlyExit("MessageTemplateNotFound", LLSD().with("PATH", message_template_path));
		}

		if(gMessageSystem && gMessageSystem->isOK())
		{
			// Initialize all of the callbacks in case of bad message
			// system data
			LLMessageSystem* msg = gMessageSystem;
			msg->setExceptionFunc(MX_UNREGISTERED_MESSAGE,
								  invalid_message_callback,
								  NULL);
			msg->setExceptionFunc(MX_PACKET_TOO_SHORT,
								  invalid_message_callback,
								  NULL);

			// running off end of a packet is now valid in the case
			// when a reader has a newer message template than
			// the sender
			/*msg->setExceptionFunc(MX_RAN_OFF_END_OF_PACKET,
								  invalid_message_callback,
								  NULL);*/
			msg->setExceptionFunc(MX_WROTE_PAST_BUFFER_SIZE,
								  invalid_message_callback,
								  NULL);

			if (gSavedSettings.getBOOL("LogMessages"))
			{
				LL_DEBUGS("AppInit") << "Message logging activated!" << LL_ENDL;
				msg->startLogging();
			}

			// start the xfer system. by default, choke the downloads
			// a lot...
			const S32 VIEWER_MAX_XFER = 3;
			start_xfer_manager();
			gXferManager->setMaxIncomingXfers(VIEWER_MAX_XFER);
			F32 xfer_throttle_bps = gSavedSettings.getF32("XferThrottle");
			if (xfer_throttle_bps > 1.f)
			{
				gXferManager->setUseAckThrottling(TRUE);
				gXferManager->setAckThrottleBPS(xfer_throttle_bps);
			}
			gAssetStorage = new LLViewerAssetStorage(msg, gXferManager);


			F32 dropPercent = gSavedSettings.getF32("PacketDropPercentage");
			msg->mPacketRing.setDropPercentage(dropPercent);

            F32 inBandwidth = gSavedSettings.getF32("InBandwidth"); 
            F32 outBandwidth = gSavedSettings.getF32("OutBandwidth"); 
			if (inBandwidth != 0.f)
			{
				LL_DEBUGS("AppInit") << "Setting packetring incoming bandwidth to " << inBandwidth << LL_ENDL;
				msg->mPacketRing.setUseInThrottle(TRUE);
				msg->mPacketRing.setInBandwidth(inBandwidth);
			}
			if (outBandwidth != 0.f)
			{
				LL_DEBUGS("AppInit") << "Setting packetring outgoing bandwidth to " << outBandwidth << LL_ENDL;
				msg->mPacketRing.setUseOutThrottle(TRUE);
				msg->mPacketRing.setOutBandwidth(outBandwidth);
			}
		}

		LL_INFOS("AppInit") << "Message System Initialized." << LL_ENDL;

		// <FS:Techwolf Lupindo> load global xml data
		FSData::instance().startDownload();
		// </FS:Techwolf Lupindo>

// <AW: opensim>
#ifndef SINGLEGRID
		if(!gSavedSettings.getBOOL("GridListDownload"))
		{
			sGridListRequestReady = true;
		}
		else
		{
			std::string filename = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "grids.remote.xml");

			llstat file_stat; //platform independent wrapper for stat
			time_t last_modified = 0;

			if(!LLFile::stat(filename, &file_stat))//exists
			{
				last_modified = file_stat.st_mtime;
			}

			std::string url = gSavedSettings.getString("GridListDownloadURL");
			FSCoreHttpUtil::callbackHttpGet(url, last_modified, boost::bind(downloadGridlistComplete, _1), boost::bind(downloadGridlistError, _1, url));
		}
#else
		sGridListRequestReady = true;
#endif

#ifdef OPENSIM // <FS:AW optional opensim support>
		// Fetch grid infos as needed
		LLGridManager::getInstance()->initGrids();
		LLStartUp::setStartupState( STATE_FETCH_GRID_INFO );
// <FS:AW optional opensim support>
#else
		LLGridManager::getInstance()->initialize(std::string());
		// <FS:Techwolf Lupindo> fsdata support
		//LLStartUp::setStartupState( STATE_AUDIO_INIT );
		LLStartUp::setStartupState( STATE_FETCH_GRID_INFO );
		// </FS:Techwolf Lupindo>
#endif // OPENSIM 
// </FS:AW optional opensim support>
	}

	if (STATE_FETCH_GRID_INFO == LLStartUp::getStartupState())
	{
 // <FS:AW optional opensim support>
		static LLFrameTimer grid_timer;

		const F32 grid_time = grid_timer.getElapsedTimeF32();
		const F32 MAX_WAIT_TIME = 15.f;//don't wait forever

		if(grid_time > MAX_WAIT_TIME ||
#ifdef OPENSIM
			( sGridListRequestReady && LLGridManager::getInstance()->isReadyToLogin() &&
#endif 		// <FS:Techwolf Lupindo> fsdata support
		    FSData::instance().getFSDataDone())
#ifdef OPENSIM
						      )
#endif		// </FS:Techwolf Lupindo>
		{
			LLStartUp::setStartupState( STATE_AUDIO_INIT );
		}
		else
		{
			ms_sleep(1);
			return FALSE;
		}
// <FS:AW optional opensim support>
	}

	if (STATE_AUDIO_INIT == LLStartUp::getStartupState())
	{

		// parsing slurls depending on the grid obviously 
		// only works after we have a grid list
		// Anyway this belongs into the gridmanager as soon as 
		// it is cleaner
		
		if(!LLStartUp::getStartSLURLString().empty())
		{
			LLStartUp::setStartSLURL(LLStartUp::getStartSLURLString());
		}		
// </AW: opensim>

		//-------------------------------------------------
		// Init audio, which may be needed for prefs dialog
		// or audio cues in connection UI.
		//-------------------------------------------------

		if (FALSE == gSavedSettings.getBOOL("NoAudio"))
		{
			delete gAudiop;
			gAudiop = NULL;

#ifdef LL_FMODSTUDIO
#if !LL_WINDOWS
            if (NULL == getenv("LL_BAD_FMODSTUDIO_DRIVER"))
#endif // !LL_WINDOWS
            {
                gAudiop = (LLAudioEngine *) new LLAudioEngine_FMODSTUDIO(gSavedSettings.getBOOL("FMODProfilerEnable"), gSavedSettings.getU32("FMODResampleMethod"));
            }
#endif

#ifdef LL_OPENAL
#if !LL_WINDOWS
			// if (NULL == getenv("LL_BAD_OPENAL_DRIVER"))
			if (!gAudiop && NULL == getenv("LL_BAD_OPENAL_DRIVER"))
#endif // !LL_WINDOWS
			{
				gAudiop = (LLAudioEngine *) new LLAudioEngine_OpenAL();
			}
#endif
            
			if (gAudiop)
			{
#if LL_WINDOWS
				// FMOD Studio and FMOD Ex on Windows needs the window handle to stop playing audio
				// when window is minimized. JC
				void* window_handle = (HWND)gViewerWindow->getPlatformWindow();
#else
				void* window_handle = NULL;
#endif
				bool init = gAudiop->init(kAUDIO_NUM_SOURCES, window_handle, LLAppViewer::instance()->getSecondLifeTitle());
				if(init)
				{
					// <FS:Ansariel> Output device selection
					gAudiop->setDevice(LLUUID(gSavedSettings.getString("FSOutputDeviceUUID")));

					gAudiop->setMuted(TRUE);
				}
				else
				{
					LL_WARNS("AppInit") << "Unable to initialize audio engine" << LL_ENDL;
					delete gAudiop;
					gAudiop = NULL;
				}

				if (gAudiop)
				{
					// if the audio engine hasn't set up its own preferred handler for streaming audio then set up the generic streaming audio implementation which uses media plugins
					if (NULL == gAudiop->getStreamingAudioImpl())
					{
						LL_INFOS("AppInit") << "Using media plugins to render streaming audio" << LL_ENDL;
						gAudiop->setStreamingAudioImpl(new LLStreamingAudio_MediaPlugins());
					}
				}
			}
		}
		
		LL_INFOS("AppInit") << "Audio Engine Initialized." << LL_ENDL;
		
		if (LLTimer::knownBadTimer())
		{
			LL_WARNS("AppInit") << "Unreliable timers detected (may be bad PCI chipset)!!" << LL_ENDL;
		}

		//
		// Log on to system
		//
		if (gUserCredential.isNull())
		{
			gUserCredential = gLoginHandler.initializeLoginInfo();
		}
		// Previous initializeLoginInfo may have generated user credentials.  Re-check them.
		if (gUserCredential.isNull())
		{
			show_connect_box = TRUE;
		}
		else if (gSavedSettings.getBOOL("AutoLogin"))  
		{
			// Log into last account
			// <FS:Ansariel> Option to not save password if using login cmdline switch;
			//               gLoginHandler.initializeLoginInfo() sets AutoLogin to TRUE,
			//               so we end up here!
			//gRememberPassword = true;
			//gRememberUser = true;
			//gSavedSettings.setBOOL("RememberPassword", TRUE);                                                      
			//gRememberUser = gSavedSettings.getBOOL("RememberUser");
			if (gSavedSettings.getBOOL("FSLoginDontSavePassword"))
			{
				gRememberPassword = false;
			}
			else
			{
				gRememberPassword = true;
				gSavedSettings.setBOOL("RememberPassword", TRUE);
			}
			// </FS:Ansariel>

			show_connect_box = false;    			
		}
		// <FS:Ansariel> Handled via FSLoginDontSavePassword debug setting
		//else if (gSavedSettings.getLLSD("UserLoginInfo").size() == 3)
		//{
		//	// Console provided login&password
		//	gRememberPassword = gSavedSettings.getBOOL("RememberPassword");
		//	gRememberUser = gSavedSettings.getBOOL("RememberUser");
		//	show_connect_box = false;
		//}
		// </FS:Ansariel>
		else 
		{
			gRememberPassword = gSavedSettings.getBOOL("RememberPassword");
			gRememberUser = gSavedSettings.getBOOL("RememberUser");
			show_connect_box = TRUE;
		}

// [RLVa:KB] - Patch: RLVa-2.1.0
		if (gSavedSettings.get<bool>(RlvSettingNames::Main))
		{
			show_connect_box = TRUE;
		}
// [/RVA:KB]

		//setup map of datetime strings to codes and slt & local time offset from utc
		// *TODO: Does this need to be here?
		LLStringOps::setupDatetimeInfo(false);

		// <FS:Beq> [FIRE-22130] for LOD Factors > 4 reset to the detected dafault 
		if (gSavedSettings.getF32("RenderVolumeLODFactor") > 4.f)
		{
			bool feature_table_success = false;
			LLFeatureManager& feature_manager = LLFeatureManager::instance();
			U32 level = gSavedSettings.getU32("RenderQualityPerformance");
			if (feature_manager.isValidGraphicsLevel(level))
			{
				std::string level_name = feature_manager.getNameForGraphicsLevel(level);
				LLFeatureList* feature_list = feature_manager.findMask(level_name);
				if (feature_list)
				{
					F32 new_val = feature_list->getRecommendedValue("RenderVolumeLODFactor");
					if (new_val > 0.f && new_val <= 4.f)
					{
						feature_table_success = true;
						gSavedSettings.setF32("RenderVolumeLODFactor", new_val);
						LL_INFOS("AppInit") << "LOD Factor too high. Resetting to recommended value for graphics level '" << level_name << "': " << new_val << LL_ENDL;
					}
				}
			}

			if (!feature_table_success)
			{
				gSavedSettings.getControl("RenderVolumeLODFactor")->resetToDefault(true);
				if (gSavedSettings.getF32("RenderVolumeLODFactor") > 4.f)
				{
					gSavedSettings.setF32("RenderVolumeLODFactor", 2.f);
				}
				LL_INFOS("AppInit") << "LOD Factor too high. Resetting to recommended value for global default: " << gSavedSettings.getF32("RenderVolumeLODFactor") << LL_ENDL;
			}
		}
		// </FS:Beq>

		// Go to the next startup state
		LLStartUp::setStartupState( STATE_BROWSER_INIT );
		return FALSE;
	}

	
	if (STATE_BROWSER_INIT == LLStartUp::getStartupState())
	{
		LL_DEBUGS("AppInit") << "STATE_BROWSER_INIT" << LL_ENDL;
		std::string msg = LLTrans::getString("LoginInitializingBrowser");
		set_startup_status(0.03f, msg.c_str(), gAgent.mMOTD.c_str());
		display_startup();
		// LLViewerMedia::initBrowser();
		LLStartUp::setStartupState( STATE_LOGIN_SHOW );
		return FALSE;
	}


	if (STATE_LOGIN_SHOW == LLStartUp::getStartupState())
	{
		LL_DEBUGS("AppInit") << "Initializing Window, show_connect_box = "
							 << show_connect_box << LL_ENDL;

		// if we've gone backwards in the login state machine, to this state where we show the UI
		// AND the debug setting to exit in this case is true, then go ahead and bail quickly
		if ( mLoginStatePastUI && gSavedSettings.getBOOL("QuitOnLoginActivated") )
		{
			LL_DEBUGS("AppInit") << "taking QuitOnLoginActivated exit" << LL_ENDL;
			// no requirement for notification here - just exit
			LLAppViewer::instance()->earlyExitNoNotify();
		}

		gViewerWindow->getWindow()->setCursor(UI_CURSOR_ARROW);

		// Login screen needs menus for preferences, but we can enter
		// this startup phase more than once.
		if (gLoginMenuBarView == NULL)
		{
			// <FS:Zi> Moved this to initBase() in llviewerwindow.cpp to get the edit menu set up
			//         before any text widget uses it
			// initialize_spellcheck_menu();
			// </FS:Zi>
			init_menus();
		}
		show_release_notes_if_required();

		if (show_connect_box)
		{
			LL_DEBUGS("AppInit") << "show_connect_box on" << LL_ENDL;
			// Load all the name information out of the login view
			// NOTE: Hits "Attempted getFields with no login view shown" warning, since we don't
			// show the login view until login_show() is called below.  
			if (gUserCredential.isNull())                                                                          
			{                                                  
				LL_DEBUGS("AppInit") << "loading credentials from gLoginHandler" << LL_ENDL;
				gUserCredential = gLoginHandler.initializeLoginInfo();                 
			}     
			// Make sure the process dialog doesn't hide things
			gViewerWindow->setShowProgress(FALSE,FALSE);
			// Show the login dialog
			login_show();
			// connect dialog is already shown, so fill in the names
			// <FS:CR>
			//LLPanelLogin::populateFields( gUserCredential, gRememberUser, gRememberPassword);
			if (gUserCredential.notNull() && !FSPanelLogin::isCredentialSet())
			{
				FSPanelLogin::setFields(gUserCredential, true);
			}
			// </FS:CR>
			// <FS:Ansariel> [FS Login Panel]
			//LLPanelLogin::giveFocus();
			FSPanelLogin::giveFocus();
			// </FS:Ansariel> [FS Login Panel]

			// MAINT-3231 Show first run dialog only for Desura viewer
			if (gSavedSettings.getString("sourceid") == "1208_desura")
			{
				if (gSavedSettings.getBOOL("FirstLoginThisInstall"))
				{
					LL_INFOS("AppInit") << "FirstLoginThisInstall, calling show_first_run_dialog()" << LL_ENDL;
				// <FS:CR> Don't show first run dialog, ever, at all.
				//	show_first_run_dialog();
				}
				else
				{
					LL_DEBUGS("AppInit") << "FirstLoginThisInstall off" << LL_ENDL;
				}
			}
			display_startup();
			LLStartUp::setStartupState( STATE_LOGIN_WAIT );		// Wait for user input
		}
		else
		{
			LL_DEBUGS("AppInit") << "show_connect_box off, skipping to STATE_LOGIN_CLEANUP" << LL_ENDL;
			// skip directly to message template verification
			LLStartUp::setStartupState( STATE_LOGIN_CLEANUP );
		}

		gViewerWindow->setNormalControlsVisible( FALSE );	
		gLoginMenuBarView->setVisible( TRUE );
		gLoginMenuBarView->setEnabled( TRUE );
		
		LLNotificationsUI::LLScreenChannelBase* chat_channel = LLNotificationsUI::LLChannelManager::getInstance()->findChannelByID(LLUUID(gSavedSettings.getString("NearByChatChannelUUID")));
		if(chat_channel)
		{
			chat_channel->removeToastsFromChannel();
		}

		show_debug_menus();

		// Hide the splash screen
		LL_DEBUGS("AppInit") << "Hide the splash screen and show window" << LL_ENDL;
		LLSplashScreen::hide();
		// Push our window frontmost
		gViewerWindow->getWindow()->show();

		// DEV-16927.  The following code removes errant keystrokes that happen while the window is being 
		// first made visible.
#ifdef _WIN32
        LL_DEBUGS("AppInit") << "Processing PeekMessage" << LL_ENDL;
		MSG msg;
		while( PeekMessage( &msg, /*All hWnds owned by this thread */ NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) )
        {
        }
        LL_DEBUGS("AppInit") << "PeekMessage processed" << LL_ENDL;
#endif
        display_startup();
        timeout.reset();
		return FALSE;
	}

	if (STATE_LOGIN_WAIT == LLStartUp::getStartupState())
	{
		// when we get to this state, we've already been past the login UI
		// (possiblely automatically) - flag this so we can test in the 
		// STATE_LOGIN_SHOW state if we've gone backwards
		mLoginStatePastUI = true;

		// Don't do anything.  Wait for the login view to call the login_callback,
		// which will push us to the next state.

		// display() function will be the one to run display_startup()
		// Sleep so we don't spin the CPU
		ms_sleep(1);
		return FALSE;
	}

	if (STATE_LOGIN_CLEANUP == LLStartUp::getStartupState())
	{
		// <FS:Ansariel> Check for test build expiration
		if (is_testbuild_expired())
		{
			LL_INFOS() << "This test version has expired and cannot be used any further." << LL_ENDL;
			LLNotificationsUtil::add("TestversionExpired", LLSD(), LLSD(), login_alert_done);
			LLStartUp::setStartupState(STATE_LOGIN_CONFIRM_NOTIFICATON);
			show_connect_box = true;
			return FALSE;
		}
		// </FS:Ansariel>

		// <FS:Ansariel> Login block
		LLSD blocked = FSData::instance().allowedLogin();
		if (blocked.isMap()) //hack for testing for an empty LLSD
		{
			LLNotificationsUtil::add("BlockLoginInfo", blocked, LLSD(), login_alert_done);
			LLStartUp::setStartupState(STATE_LOGIN_CONFIRM_NOTIFICATON);
			show_connect_box = true;
			return FALSE;
		}
		// </FS:Ansariel>

		// Post login screen, we should see if any settings have changed that may
		// require us to either start/stop or change the socks proxy. As various communications
		// past this point may require the proxy to be up.
		if (!LLStartUp::startLLProxy())
		{
			// Proxy start up failed, we should now bail the state machine
			// startLLProxy() will have reported an error to the user
			// already, so we just go back to the login screen. The user
			// could then change the preferences to fix the issue.

			LLStartUp::setStartupState(STATE_LOGIN_SHOW);
			return FALSE;
		}

// [RLVa:KB] - Checked: RLVa-0.2.1
		if (gSavedSettings.get<bool>(RlvSettingNames::Main))
		{
			RlvHandler::setEnabled(true);
		}
// [/RLVa:KB]

		// reset the values that could have come in from a slurl
		// DEV-42215: Make sure they're not empty -- gUserCredential
		// might already have been set from gSavedSettings, and it's too bad
		// to overwrite valid values with empty strings.

		if (show_connect_box)
		{
			// TODO if not use viewer auth
			// Load all the name information out of the login view
			// <FS:Ansariel> [FS Login Panel]
			//LLPanelLogin::getFields(gUserCredential, gRememberUser, gRememberPassword);
			FSPanelLogin::getFields(gUserCredential, gRememberPassword); 
			// </FS:Ansariel> [FS Login Panel]
			// end TODO
	 
			// HACK: Try to make not jump on login
			gKeyboard->resetKeys();
		}

		// when we get to this state, we've already been past the login UI
		// (possiblely automatically) - flag this so we can test in the 
		// STATE_LOGIN_SHOW state if we've gone backwards
		mLoginStatePastUI = true;

		// save the credentials
		std::string userid = "unknown";
        if (gUserCredential.notNull())
        {
            userid = gUserCredential->userID();
            // <FS:Ansariel> [FS Login Panel]
            //if (gRememberUser)
            //{
            //    gSecAPIHandler->addToCredentialMap("login_list", gUserCredential, gRememberPassword);
            //    // Legacy viewers use this method to store user credentials, newer viewers
            //    // reuse it to be compatible and to remember last session
            //    gSecAPIHandler->saveCredential(gUserCredential, gRememberPassword);
            //}
            // </FS:Ansariel> [FS Login Panel]
        }
		// <FS:Ansariel> Option to not save password if using login cmdline switch
		//gSavedSettings.setBOOL("RememberPassword", gRememberPassword);                                                 
		//gSavedSettings.setBOOL("RememberUser", gRememberUser);
		if (!gSavedSettings.getBOOL("FSLoginDontSavePassword"))
		{
			gSavedSettings.setBOOL("RememberPassword", gRememberPassword);
		}
		// </FS:Ansariel>
		LL_INFOS("AppInit") << "Attempting login as: " << userid << LL_ENDL;                                           
//		gDebugInfo["LoginName"] = userid;                                                                              
// [SL:KB] - Patch: Viewer-CrashReporting | Checked: 2010-11-16 (Catznip-2.6.0a) | Added: Catznip-2.4.0b
		if (gCrashSettings.getBOOL("CrashSubmitName"))
		{
			// Only include the agent name if the user consented
			gDebugInfo["LoginName"] = userid;                                                                              
		}
// [/SL:KB]

		// We don't save this version of the title because it'll
		//  be replaced later, we hope. -- TS
		size_t underscore_pos = userid.find_first_of('_');
		std::string display_id = userid.substr(0,underscore_pos);
		if ((underscore_pos != std::string::npos) &&
			(underscore_pos < userid.length()-1))
		{
			std::string id_last = userid.substr(underscore_pos+1);
			if (id_last.compare("Resident") != 0)
			{
				display_id = display_id + " " + id_last;
			}
		}
		std::string window_title = gWindowTitle + " - " + display_id;
		LLStringUtil::truncate(window_title, 255);
		gViewerWindow->setTitle(window_title);

		// create necessary directories
		// *FIX: these mkdir's should error check
// <FS:CR> Seperate user directories per grid
		//gDirUtilp->setLindenUserDir(userid, gridlabel);
		std::string gridlabel = LLGridManager::getInstance()->getGridLabel();
		gDirUtilp->setLindenUserDir(userid, LLGridManager::getInstance()->getGridLabel());
// </FS:CR>
		LLFile::mkdir(gDirUtilp->getLindenUserDir());

		// As soon as directories are ready initialize notification storages
		if (!LLPersistentNotificationStorage::instanceExists())
		{
			// check existance since this part of code can be reached
			// twice due to login failures
			LLPersistentNotificationStorage::initParamSingleton();
			LLDoNotDisturbNotificationStorage::initParamSingleton();
		}

		// Set PerAccountSettingsFile to the default value.
		std::string settings_per_account = gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, LLAppViewer::instance()->getSettingsFilename("Default", "PerAccount"));
		gSavedSettings.setString("PerAccountSettingsFile", settings_per_account);
		gDebugInfo["PerAccountSettingsFilename"] = settings_per_account;

		// Note: can't store warnings files per account because some come up before login
		
		// Overwrite default user settings with user settings								 
		LLAppViewer::instance()->loadSettingsFromDirectory("Account");

		// <FS:Ansariel> Restore bottom toolbar layout now he have the user settings
		LLLayoutStack* chat_bar_stack = gToolBarView->findChild<LLLayoutStack>("chat_bar_stack");
		if (chat_bar_stack)
		{
			chat_bar_stack->refreshFromSettings();
		}
		// </FS:Ansariel>

		// Convert 'LogInstantMessages' into 'KeepConversationLogTranscripts' for backward compatibility (CHUI-743).
		// <FS:CR> FIRE-11410 - Don't do this, handle it in settings restore and first run
		//LLControlVariablePtr logInstantMessagesControl = gSavedPerAccountSettings.getControl("LogInstantMessages");
		//if (logInstantMessagesControl.notNull())
		//{
		//	gSavedPerAccountSettings.setS32("KeepConversationLogTranscripts", logInstantMessagesControl->getValue() ? 2 : 1);
		//}

		// Need to set the LastLogoff time here if we don't have one.  LastLogoff is used for "Recent Items" calculation
		// and startup time is close enough if we don't have a real value.
		if (gSavedPerAccountSettings.getU32("LastLogoff") == 0)
		{
			gSavedPerAccountSettings.setU32("LastLogoff", time_corrected());
		}

		//Default the path if one isn't set.
		// *NOTE: unable to check variable differ from "InstantMessageLogPath" because it was
		// provided in pre 2.0 viewer. See EXT-6661
		if (gSavedPerAccountSettings.getString("InstantMessageLogPath").empty())
		{
			gDirUtilp->setChatLogsDir(gDirUtilp->getOSUserAppDir());
			gSavedPerAccountSettings.setString("InstantMessageLogPath", gDirUtilp->getChatLogsDir());
		}
		else
		{
			gDirUtilp->setChatLogsDir(gSavedPerAccountSettings.getString("InstantMessageLogPath"));		
		}
		// <FS:KC> FIRE-18247: Handle non-existent chat log location
		if (!gDirUtilp->fileExists(gSavedPerAccountSettings.getString("InstantMessageLogPath")))
		{
			gDirUtilp->setChatLogsDir(gDirUtilp->getOSUserAppDir());
			gSavedPerAccountSettings.setString("InstantMessageLogPath", gDirUtilp->getChatLogsDir());
		}
		// </FS:KC>

		// <FS:LO> FIRE-22853 Make snapshots to disk not remember path and filename if the user doesnt want them to.
		if (!gSavedPerAccountSettings.getBOOL("FSRememberSnapshotPathSessions"))
		{
			gSavedPerAccountSettings.setString("SnapshotBaseDir", gSavedPerAccountSettings.getControl("SnapshotBaseDir")->getDefault().asString());
			gSavedPerAccountSettings.setString("SnapshotBaseName", gSavedPerAccountSettings.getControl("SnapshotBaseName")->getDefault().asString());
		}
		// </FS:LO>
// <FS:CR> Seperate user directories per grid
		//gDirUtilp->setPerAccountChatLogsDir(userid);
		gDirUtilp->setPerAccountChatLogsDir(userid, gridlabel);
// </FS:CR>
		LLFile::mkdir(gDirUtilp->getChatLogsDir());
		LLFile::mkdir(gDirUtilp->getPerAccountChatLogsDir());

		// NaCl - Antispam
		NACLAntiSpamRegistry::instance();
		// NaCl End
		// <FS:Ansariel> Create user fonts directory
		std::string user_fonts_path_name(gDirUtilp->getExpandedFilename( LL_PATH_USER_SETTINGS , "fonts", ""));
		LLFile::mkdir(user_fonts_path_name.c_str());

		// <FS:WS> Initalize Account based asset_blacklist
		FSAssetBlacklist::getInstance()->init();

		// <FS:Techwolf Lupindo> load per grid data
		FSData::instance().downloadAgents();
		// </FS:Techwolf Lupindo>
		if (show_connect_box)
		{
			LLSLURL slurl;
			// WS: Close the Panel only, if we have DisableLoginScreens enabled. Else fade away.
			if(gSavedSettings.getBOOL("FSDisableLoginScreens"))
				// <FS:Ansariel> [FS Login Panel]
				//LLPanelLogin::closePanel();
				FSPanelLogin::closePanel();
				// </FS:Ansariel> [FS Login Panel]
		}

		
		// Load URL History File
		LLURLHistory::loadFile("url_history.xml");
		// Load location history 
		LLLocationHistory::getInstance()->load();

		// <FS:Ansariel> FIRE-10607: Avatar icon controls show wrong picture when switching between SL main/beta grid
		// Moved further down until we know what grid we are connecting to
		// Load Avatars icons cache
		//LLAvatarIconIDCache::getInstance()->load();
		// </FS:Ansariel>
		
		// <FS:Ansariel> [FS Persisted Avatar Render Settings]
		//LLRenderMuteList::getInstance()->loadFromFile();

		//-------------------------------------------------
		// Handle startup progress screen
		//-------------------------------------------------

		// on startup the user can request to go to their home,
		// their last location, or some URL "-url //sim/x/y[/z]"
		// All accounts have both a home and a last location, and we don't support
		// more locations than that.  Choose the appropriate one.  JC
// [RLVa:KB] - Checked: RLVa-0.2.1
		if ( (RlvHandler::isEnabled()) && (RlvSettings::getLoginLastLocation()) )
		{
			// Force login at the last location
			LLStartUp::setStartSLURL(LLSLURL(LLSLURL::SIM_LOCATION_LAST));
		}
// [/RLVa:KB]

		switch (LLStartUp::getStartSLURL().getType())
		  {
		  case LLSLURL::LOCATION:
		    agent_location_id = START_LOCATION_ID_URL;
		    break;
		  case LLSLURL::LAST_LOCATION:
		    agent_location_id = START_LOCATION_ID_LAST;
		    break;
		  default:
		    agent_location_id = START_LOCATION_ID_HOME;
		    break;
		  }

		gViewerWindow->getWindow()->setCursor(UI_CURSOR_WAIT);

		// Display the startup progress bar.
		gViewerWindow->initTextures(agent_location_id);
		gViewerWindow->setShowProgress(TRUE,!gSavedSettings.getBOOL("FSDisableLoginScreens"));
		gViewerWindow->setProgressCancelButtonVisible(TRUE, LLTrans::getString("Quit"));

		gViewerWindow->revealIntroPanel();

		// Ansariel: Update viewer help menu; Needed if logging
		//           in by autologin or on SL it would show
		//           the non-functional "About [CURRENT_GRID]"
		//           and "[CURRENT_GRID] Help" menu entries
		update_grid_help();

		// <FS:Techwolf Lupindo> fsdata agents support
		//LLStartUp::setStartupState( STATE_LOGIN_AUTH_INIT );
		LLStartUp::setStartupState(STATE_AGENTS_WAIT);
		// </FS:Techwolf Lupindo>

		return FALSE;
	}

	// <FS:Techwolf Lupindo> fsdata support
	if (STATE_AGENTS_WAIT == LLStartUp::getStartupState())
	{
		static LLFrameTimer agents_timer;
		const F32 agents_time = agents_timer.getElapsedTimeF32();
		const F32 MAX_AGENTS_TIME = 15.f;

		if(agents_time > MAX_AGENTS_TIME || FSData::instance().getAgentsDone())
		{
			LLStartUp::setStartupState(STATE_LOGIN_AUTH_INIT);
		}
		else
		{
			ms_sleep(1);
			return FALSE;
		}
	}
	// </FS:Techwolf Lupindo>
	
	if(STATE_LOGIN_AUTH_INIT == LLStartUp::getStartupState())
	{
		gDebugInfo["GridName"] = LLGridManager::getInstance()->getGridId();

		// Update progress status and the display loop.
		auth_desc = LLTrans::getString("LoginInProgress");
		set_startup_status(progress, auth_desc, auth_message);
		progress += 0.02f;
		display_startup();

// <AW: crash report grid correctness>
		eLastExecEvent last_exec_event = gLastExecEvent;
		const std::string current_grid = LLGridManager::getInstance()->getGrid();
		const std::string last_grid = gSavedSettings.getString("LastConnectedGrid");
		if (!last_grid.empty() && last_grid != current_grid)
		{
			// don't report crashes on a different grid than the one connecting to,
			// since a bad OpenSim setup can crash the viewer a lot
			last_exec_event = LAST_EXEC_NORMAL;
		}
		LLTrans::setDefaultArg("CURRENT_GRID", LLGridManager::getInstance()->getGridLabel());
// </AW: crash report grid correctness>

		// Setting initial values...
		LLLoginInstance* login = LLLoginInstance::getInstance();
		login->setNotificationsInterface(LLNotifications::getInstance());

		login->setSerialNumber(LLAppViewer::instance()->getSerialNumber());
// <AW: crash report grid correctness>
//		login->setLastExecEvent(gLastExecEvent);
		login->setLastExecDuration(gLastExecDuration);
		login->setLastExecEvent(last_exec_event);
// </AW: crash report grid correctness>


		// This call to LLLoginInstance::connect() starts the 
		// authentication process.
		login->connect(gUserCredential);
#if defined(OPENSIM) && !defined(SINGLEGRID) // <FS:AW optional opensim support>
// <AW: opensim>
		LLGridManager::getInstance()->saveGridList();
// </AW: opensim>
#endif // OPENSIM // <FS:AW optional opensim support>
		LLStartUp::setStartupState( STATE_LOGIN_CURL_UNSTUCK );
		return FALSE;
	}

	if(STATE_LOGIN_CURL_UNSTUCK == LLStartUp::getStartupState())
	{
		// If we get here we have gotten past the potential stall
		// in curl, so take "may appear frozen" out of progress bar. JC
		auth_desc = LLTrans::getString("LoginInProgressNoFrozen");
		set_startup_status(progress, auth_desc, auth_message);

		LLStartUp::setStartupState( STATE_LOGIN_PROCESS_RESPONSE );
		return FALSE;
	}

	if(STATE_LOGIN_PROCESS_RESPONSE == LLStartUp::getStartupState()) 
	{
		// Generic failure message
		std::ostringstream emsg;
		emsg << LLTrans::getString("LoginFailed") << "\n";
		if(LLLoginInstance::getInstance()->authFailure())
		{
			LL_INFOS("LLStartup") << "Login failed, LLLoginInstance::getResponse(): "
			                      << LLLoginInstance::getInstance()->getResponse() << LL_ENDL;
			LLSD response = LLLoginInstance::getInstance()->getResponse();
			// Still have error conditions that may need some 
			// sort of handling - dig up specific message
			std::string reason_response = response["reason"];
			std::string message_response = response["message"];
			std::string message_id = response["message_id"];
			std::string message; // actual string to show the user

			if(!message_id.empty() && LLTrans::findString(message, message_id, response["message_args"]))
			{
				// message will be filled in with the template and arguments
			}
			else if(!message_response.empty())
			{
				// *HACK: "no_inventory_host" sent as the message itself.
				// Remove this clause when server is sending message_id as well.
				message = LLAgent::sTeleportErrorMessages[ message_response ];
			}

			if (message.empty())
			{
				// Fallback to server-supplied string; necessary since server
				// may add strings that this viewer is not yet aware of
				message = message_response;
			}

			emsg << message;


			if(reason_response == "key")
			{
				// Couldn't login because user/password is wrong
				// Clear the credential
				gUserCredential->clearAuthenticator();
			}

			if(reason_response == "update" 
				|| reason_response == "optional")
			{
				// In the case of a needed update, quit.
				// Its either downloading or declined.
				// If optional was skipped this case shouldn't 
				// be reached.

				LL_INFOS("LLStartup") << "Forcing a quit due to update." << LL_ENDL;
				LLLoginInstance::getInstance()->disconnect();
				LLAppViewer::instance()->forceQuit();
			}
			else 
			{
				if (reason_response != "tos") 
				{
					// Don't pop up a notification in the TOS case because
					// LLFloaterTOS::onCancel() already scolded the user.
					std::string error_code;
					if(response.has("errorcode"))
					{
						error_code = response["errorcode"].asString();
					}
					if ((reason_response == "CURLError") && 
						(error_code == "SSL_CACERT" || error_code == "SSL_PEER_CERTIFICATE") && 
						response.has("certificate"))
					{
						// This was a certificate error, so grab the certificate
						// and throw up the appropriate dialog.
                        LLPointer<LLCertificate> certificate;
                        try
                        {
                            certificate = gSecAPIHandler->getCertificate(response["certificate"]);
                        }
                        catch (LLCertException &cert_exception)
                        {
                            LL_WARNS("LLStartup", "SECAPI") << "Caught " << cert_exception.what() << " certificate expception on getCertificate("<< response["certificate"] << ")" << LL_ENDL;
                            LLSD args;
                            args["REASON"] = LLTrans::getString(cert_exception.what());

                            LLNotificationsUtil::add("GeneralCertificateErrorShort", args, response,
                                general_cert_done);

                            reset_login();
                            gSavedSettings.setBOOL("AutoLogin", FALSE);
                            show_connect_box = true;
                        }
						if(certificate)
						{
							LLSD args = transform_cert_args(certificate);

							if(error_code == "SSL_CACERT")
							{
								// if we are handling an untrusted CA, throw up the dialog                             
								// with the 'trust this CA' button.                                                    
								LLNotificationsUtil::add("TrustCertificateError", args, response,
														trust_cert_done);
								
								// <FS:Ansariel> Not needed here - done below
								//show_connect_box = true;
							}
							else
							{
								// the certificate exception returns a unique string for each type of exception.       
								// we grab this string via the LLUserAuth object, and use that to grab the localized   
								// string.                                                                             
								args["REASON"] = LLTrans::getString(message_response);
								
								LLNotificationsUtil::add("GeneralCertificateError", args, response,
														 general_cert_done);
								
								// <FS:Ansariel> Not needed here - done below & in transition_back_to_login_panel()
								//reset_login();
								//gSavedSettings.setBOOL("AutoLogin", FALSE);
								//show_connect_box = true;
								// </FS:Ansariel>
								
							}

						}
					}
					else if (!message.empty())
					{
						// This wasn't a certificate error, so throw up the normal
						// notificatioin message.
						LLSD args;
						args["ERROR_MESSAGE"] = emsg.str();
						LL_INFOS("LLStartup") << "Notification: " << args << LL_ENDL;
						LLNotificationsUtil::add("ErrorMessage", args, LLSD(), login_alert_done);
					}
				}
				// <FS:Ansariel> Wait for notification confirmation
				//transition_back_to_login_panel(emsg.str());
				LLStartUp::setStartupState(STATE_LOGIN_CONFIRM_NOTIFICATON);
				// </FS:Ansariel>
				show_connect_box = true;
			}
		}
		else if(LLLoginInstance::getInstance()->authSuccess())
		{
// <FS:CR> Aurora Sim
			//if(process_login_success_response())
			if(process_login_success_response(first_sim_size_x,first_sim_size_y))
// </FS:CR> Aurora Sim
			{
// <AW: crash report grid correctness>
				const std::string current_grid = LLGridManager::getInstance()->getGrid();
				gSavedSettings.setString("LastConnectedGrid", current_grid);
// </AW: crash report grid correctness>

				// Pass the user information to the voice chat server interface.
				LLVoiceClient::getInstance()->userAuthorized(gUserCredential->userID(), gAgentID);
				// create the default proximal channel
				LLVoiceChannel::initClass();
				
				if (gSavedSettings.getBOOL("FSRememberUsername"))
				{
					gSecAPIHandler->saveCredential(gUserCredential, gRememberPassword);
				}
				FSPanelLogin::clearPassword();
				LLStartUp::setStartupState( STATE_WORLD_INIT);
				LLTrace::get_frame_recording().reset();
			}
			else
			{
				LLSD args;
				args["ERROR_MESSAGE"] = emsg.str();
				LL_INFOS("LLStartup") << "Notification: " << args << LL_ENDL;
				LLNotificationsUtil::add("ErrorMessage", args, LLSD(), login_alert_done);
				// <FS:Ansariel> Wait for notification confirmation
				//transition_back_to_login_panel(emsg.str());
				LLStartUp::setStartupState(STATE_LOGIN_CONFIRM_NOTIFICATON);
				// </FS:Ansariel>
				show_connect_box = true;
				return FALSE;
			}
		}
		return FALSE;
	}

	// <FS:Ansariel> Wait for notification confirmation
	if (STATE_LOGIN_CONFIRM_NOTIFICATON == LLStartUp::getStartupState())
	{
		display_startup();
		gViewerWindow->getProgressView()->setVisible(FALSE);
		display_startup();
		ms_sleep(1);
		return FALSE;
	}
	// </FS:Ansariel>

	//---------------------------------------------------------------------
	// World Init
	//---------------------------------------------------------------------
	if (STATE_WORLD_INIT == LLStartUp::getStartupState())
	{
		set_startup_status(0.30f, LLTrans::getString("LoginInitializingWorld"), gAgent.mMOTD);
		display_startup();
		// We should have an agent id by this point.
		llassert(!(gAgentID == LLUUID::null));

		// <FS:Ansariel> Force HTTP inventory enabled on Second Life
#ifdef OPENSIM
		if (LLGridManager::getInstance()->isInSecondLife())
#endif
		{
			gMenuBarView->getChild<LLMenuItemGL>("HTTP Textures")->setVisible(FALSE);
			gMenuBarView->getChild<LLMenuItemGL>("HTTP Inventory")->setVisible(FALSE);
		}
		// </FS:Ansariel>

		// <FS:Ansariel> FIRE-10607: Avatar icon controls show wrong picture when switching between SL main/beta grid
		// Load Avatars icons cache - once we know the grid we are connecting to
		LLAvatarIconIDCache::getInstance()->load();
		// </FS:Ansariel>

		// <FS:Ansariel> Restore original LLMessageSystem HTTP options for OpenSim
		gMessageSystem->setIsInSecondLife(LLGridManager::getInstance()->isInSecondLife());

		// Finish agent initialization.  (Requires gSavedSettings, builds camera)
		gAgent.init();
		display_startup();
		gAgentCamera.init();
		display_startup();
		display_startup();

		// Since we connected, save off the settings so the user doesn't have to
		// type the name/password again if we crash.
		gSavedSettings.saveToFile(gSavedSettings.getString("ClientSettingsFile"), TRUE);
		LLUIColorTable::instance().saveUserSettings();

		display_startup();

		//
		// Initialize classes w/graphics stuff.
		//
		LLViewerStatsRecorder::instance(); // Since textures work in threads
		gTextureList.doPrefetchImages();		
		display_startup();

		LLSurface::initClasses();
		display_startup();

		display_startup();

		LLDrawable::initClass();
		display_startup();

		// init the shader managers
		LLPostProcess::initClass();
		display_startup();

        LLAvatarAppearance::initClass("avatar_lad.xml","avatar_skeleton.xml");
		display_startup();

		LLViewerObject::initVOClasses();
		display_startup();

		// Initialize all our tools.  Must be done after saved settings loaded.
		// NOTE: This also is where gToolMgr used to be instantiated before being turned into a singleton.
		LLToolMgr::getInstance()->initTools();
		display_startup();

		// Pre-load floaters, like the world map, that are slow to spawn
		// due to XML complexity.
		gViewerWindow->initWorldUI();
		
		display_startup();

		// This is where we used to initialize gWorldp. Original comment said:
		// World initialization must be done after above window init

		// User might have overridden far clip
		LLWorld::getInstance()->setLandFarClip(gAgentCamera.mDrawDistance);
		display_startup();
		// Before we create the first region, we need to set the agent's mOriginGlobal
		// This is necessary because creating objects before this is set will result in a
		// bad mPositionAgent cache.

		gAgent.initOriginGlobal(from_region_handle(gFirstSimHandle));
		display_startup();

// <FS:CR> Aurora Sim
		//LLWorld::getInstance()->addRegion(gFirstSimHandle, gFirstSim);
		LLWorld::getInstance()->addRegion(gFirstSimHandle, gFirstSim, first_sim_size_x, first_sim_size_y);
// </FS:CR> Aurora Sim
		display_startup();

		LLViewerRegion *regionp = LLWorld::getInstance()->getRegionFromHandle(gFirstSimHandle);
		LL_INFOS("AppInit") << "Adding initial simulator " << regionp->getOriginGlobal() << LL_ENDL;
		
		LL_DEBUGS("CrossingCaps") << "Calling setSeedCapability from init_idle(). Seed cap == "
		<< gFirstSimSeedCap << LL_ENDL;
		regionp->setSeedCapability(gFirstSimSeedCap);
		LL_DEBUGS("AppInit") << "Waiting for seed grant ...." << LL_ENDL;
		display_startup();
		// Set agent's initial region to be the one we just created.
		gAgent.setRegion(regionp);
		display_startup();
		// Set agent's initial position, which will be read by LLVOAvatar when the avatar
		// object is created.  I think this must be done after setting the region.  JC
		gAgent.setPositionAgent(agent_start_position_region);

		display_startup();
		LLStartUp::initExperiences();

		display_startup();

		// If logging should be enebled, turns it on and loads history from disk
		// Note: does not happen on init of singleton because preferences can use
		// this instance without logging in
		LLConversationLog::getInstance()->initLoggingState();

		LLStartUp::setStartupState( STATE_MULTIMEDIA_INIT );

		return FALSE;
	}


	//---------------------------------------------------------------------
	// Load QuickTime/GStreamer and other multimedia engines, can be slow.
	// Do it while we're waiting on the network for our seed capability. JC
	//---------------------------------------------------------------------
	if (STATE_MULTIMEDIA_INIT == LLStartUp::getStartupState())
	{
		LLStartUp::multimediaInit();
		LLStartUp::setStartupState( STATE_FONT_INIT );
		display_startup();
		return FALSE;
	}

	// Loading fonts takes several seconds
	if (STATE_FONT_INIT == LLStartUp::getStartupState())
	{
		LLStartUp::fontInit();
		LLStartUp::setStartupState( STATE_SEED_GRANTED_WAIT );
		display_startup();
		return FALSE;
	}

	//---------------------------------------------------------------------
	// Wait for Seed Cap Grant
	//---------------------------------------------------------------------
	if(STATE_SEED_GRANTED_WAIT == LLStartUp::getStartupState())
	{
		LLViewerRegion *regionp = LLWorld::getInstance()->getRegionFromHandle(gFirstSimHandle);
		if (regionp->capabilitiesReceived())
		{
			LLStartUp::setStartupState( STATE_SEED_CAP_GRANTED );
		}
		else
		{
			U32 num_retries = regionp->getNumSeedCapRetries();
			if (num_retries > 0)
			{
				LLStringUtil::format_map_t args;
				args["[NUMBER]"] = llformat("%d", num_retries + 1);
				set_startup_status(0.4f, LLTrans::getString("LoginRetrySeedCapGrant", args), gAgent.mMOTD.c_str());
			}
			else
			{
				set_startup_status(0.4f, LLTrans::getString("LoginRequestSeedCapGrant"), gAgent.mMOTD.c_str());
			}
		}
		display_startup();
		return FALSE;
	}


	//---------------------------------------------------------------------
	// Seed Capability Granted
	// no newMessage calls should happen before this point
	//---------------------------------------------------------------------
	if (STATE_SEED_CAP_GRANTED == LLStartUp::getStartupState())
	{
		display_startup();
		update_texture_fetch();
		display_startup();

		if ( gViewerWindow != NULL)
		{	// This isn't the first logon attempt, so show the UI
			gViewerWindow->setNormalControlsVisible( TRUE );
		}	
		gLoginMenuBarView->setVisible( FALSE );
		gLoginMenuBarView->setEnabled( FALSE );
		display_startup();

		// direct logging to the debug console's line buffer
		LLError::logToFixedBuffer(gDebugView->mDebugConsolep);
		display_startup();
		
		// set initial visibility of debug console
		gDebugView->mDebugConsolep->setVisible(gSavedSettings.getBOOL("ShowDebugConsole"));
		display_startup();

		//
		// Set message handlers
		//
		LL_INFOS("AppInit") << "Initializing communications..." << LL_ENDL;

		// register callbacks for messages. . . do this after initial handshake to make sure that we don't catch any unwanted
		register_viewer_callbacks(gMessageSystem);
		display_startup();

		// Debugging info parameters
		gMessageSystem->setMaxMessageTime( 0.5f );			// Spam if decoding all msgs takes more than 500 ms
		display_startup();

		#ifndef	LL_RELEASE_FOR_DOWNLOAD
			gMessageSystem->setTimeDecodes( TRUE );				// Time the decode of each msg
			gMessageSystem->setTimeDecodesSpamThreshold( 0.05f );  // Spam if a single msg takes over 50ms to decode
		#endif
		display_startup();

		gXferManager->registerCallbacks(gMessageSystem);
		display_startup();

		LLStartUp::initNameCache();
		display_startup();

		// update the voice settings *after* gCacheName initialization
		// so that we can construct voice UI that relies on the name cache
		if (LLVoiceClient::instanceExists())
		{
			LLVoiceClient::getInstance()->updateSettings();
		}
		display_startup();

		// <FS:Ansariel> OpenSim support: Init with defaults - we get the OpenSimExtras later during login
		LFSimFeatureHandler::instance();

		// create a container's instance for start a controlling conversation windows
		// by the voice's events
		// <FS:Ansariel> [FS communication UI]
		//LLFloaterIMContainer::getInstance();
		FSFloaterIMContainer* floater_imcontainer = FSFloaterIMContainer::getInstance();
		floater_imcontainer->initTabs();

		if (gSavedSettings.getS32("ParcelMediaAutoPlayEnable") == 2)
		{
			LLViewerParcelAskPlay::getInstance()->loadSettings();
		}

		// <FS:ND> FIRE-3066: Force creation or FSFLoaterContacts here, this way it will register with LLAvatarTracker early enough.
		// Otherwise it is only create if isChatMultriTab() == true and LLIMFloaterContainer::getInstance is called
		// Moved here from llfloaternearbyvchat.cpp by Zi, to make this work even if LogShowHistory is FALSE
		LLFloater *pContacts(FSFloaterContacts::getInstance());

		// <FS:Ansariel> Load persisted avatar render settings
		FSAvatarRenderPersistence::instance().init();
		
		// Do something with pContacts so no overzealous optimizer optimzes our neat little call to FSFloaterContacts::getInstance() away.
		if( pContacts )
			LL_INFOS("AppInit") << "Constructed " <<  pContacts->getName() << LL_ENDL;
		// </FS:ND>

		// <FS:Ansariel> FIRE-8560/FIRE-8592: We neet to create the instance of the radar
		//               for the radar functions and the V2 friendlist here.
		//               This is because of the standalone group panels that will
		//               prevent doing this at login when receiving the agent group
		//               data update.
		LLFloaterSidePanelContainer::getPanel("people", "panel_people");
		FSRadar::instance();
		LL_INFOS("AppInit") << "Radar initialized" << LL_ENDL;
		// </FS:Ansariel>

		// <FS:Ansariel> Register check function for registrar enable checks
		gFSRegistrarUtils.setEnableCheckFunction(boost::bind(&FSCommon::checkIsActionEnabled, _1, _2));

		// <FS:Techwolf Lupindo> fsdata support
		FSData::instance().addAgents();
		// </FS:Techwolf Lupindo>

		// <FS:Ansariel> [FS communication UI]
		//gCacheName is required for nearby chat history loading
		//so I just moved nearby history loading a few states further
		if (gSavedPerAccountSettings.getBOOL("LogShowHistory"))
		{
			FSFloaterNearbyChat* nearby_chat = FSFloaterNearbyChat::getInstance();
			if (nearby_chat) nearby_chat->loadHistory();
		}
		display_startup();
		// </FS:Ansariel> [FS communication UI]

		// <FS:KC> FIRE-18250: Option to disable default eye movement
		gAgent.addRegionChangedCallback(boost::bind(&update_static_eyes));
		update_static_eyes();
		// </FS:KC>

		// *Note: this is where gWorldMap used to be initialized.

		// register null callbacks for audio until the audio system is initialized
		gMessageSystem->setHandlerFuncFast(_PREHASH_SoundTrigger, null_message_callback, NULL);
		gMessageSystem->setHandlerFuncFast(_PREHASH_AttachedSound, null_message_callback, NULL);
		display_startup();

		//reset statistics
		LLViewerStats::instance().resetStats();

		display_startup();
		//
		// Set up region and surface defaults
		//


		// Sets up the parameters for the first simulator

		LL_DEBUGS("AppInit") << "Initializing camera..." << LL_ENDL;
		gFrameTime    = totalTime();
		F32Seconds last_time = gFrameTimeSeconds;
		gFrameTimeSeconds = (gFrameTime - gStartTime);

		gFrameIntervalSeconds = gFrameTimeSeconds - last_time;
		if (gFrameIntervalSeconds < 0.f)
		{
			gFrameIntervalSeconds = 0.f;
		}

		// Make sure agent knows correct aspect ratio
		// FOV limits depend upon aspect ratio so this needs to happen before initializing the FOV below
		LLViewerCamera::getInstance()->setViewHeightInPixels(gViewerWindow->getWorldViewHeightRaw());
		LLViewerCamera::getInstance()->setAspect(gViewerWindow->getWorldViewAspectRatio());
		// Initialize FOV
		LLViewerCamera::getInstance()->setDefaultFOV(gSavedSettings.getF32("CameraAngle")); 
		display_startup();

		// Move agent to starting location. The position handed to us by
		// the space server is in global coordinates, but the agent frame
		// is in region local coordinates. Therefore, we need to adjust
		// the coordinates handed to us to fit in the local region.

		gAgent.setPositionAgent(agent_start_position_region);
		gAgent.resetAxes(gAgentStartLookAt);
		gAgentCamera.stopCameraAnimation();
		gAgentCamera.resetCamera();
		display_startup();

		// Initialize global class data needed for surfaces (i.e. textures)
		LL_DEBUGS("AppInit") << "Initializing sky..." << LL_ENDL;
		// Initialize all of the viewer object classes for the first time (doing things like texture fetches.
		LLGLState::checkStates();
		LLGLState::checkTextureChannels();

		gSky.init();

		LLGLState::checkStates();
		LLGLState::checkTextureChannels();

		display_startup();

		LL_DEBUGS("AppInit") << "Decoding images..." << LL_ENDL;
		// For all images pre-loaded into viewer cache, decode them.
		// Need to do this AFTER we init the sky
		const S32 DECODE_TIME_SEC = 2;
		for (int i = 0; i < DECODE_TIME_SEC; i++)
		{
			F32 frac = (F32)i / (F32)DECODE_TIME_SEC;
			set_startup_status(0.45f + frac*0.1f, LLTrans::getString("LoginDecodingImages"), gAgent.mMOTD);
			display_startup();
			gTextureList.decodeAllImages(1.f);
		}
		LLStartUp::setStartupState( STATE_WORLD_WAIT );

		display_startup();

		// JC - Do this as late as possible to increase likelihood Purify
		// will run.
		LLMessageSystem* msg = gMessageSystem;
		if (!msg->mOurCircuitCode)
		{
			LL_WARNS("AppInit") << "Attempting to connect to simulator with a zero circuit code!" << LL_ENDL;
		}

		gUseCircuitCallbackCalled = false;

		msg->enableCircuit(gFirstSim, TRUE);
		// now, use the circuit info to tell simulator about us!
		LL_INFOS("AppInit") << "viewer: UserLoginLocationReply() Enabling " << gFirstSim << " with code " << msg->mOurCircuitCode << LL_ENDL;
		msg->newMessageFast(_PREHASH_UseCircuitCode);
		msg->nextBlockFast(_PREHASH_CircuitCode);
		msg->addU32Fast(_PREHASH_Code, msg->mOurCircuitCode);
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		msg->addUUIDFast(_PREHASH_ID, gAgent.getID());
		msg->sendReliable(
			gFirstSim,
			gSavedSettings.getS32("UseCircuitCodeMaxRetries"),
			FALSE,
			(F32Seconds)gSavedSettings.getF32("UseCircuitCodeTimeout"),
			use_circuit_callback,
			NULL);

		timeout.reset();
		display_startup();

		return FALSE;
	}

	//---------------------------------------------------------------------
	// World Wait
	//---------------------------------------------------------------------
	if(STATE_WORLD_WAIT == LLStartUp::getStartupState())
	{
		LL_DEBUGS("AppInit") << "Waiting for simulator ack...." << LL_ENDL;
		set_startup_status(0.59f, LLTrans::getString("LoginWaitingForRegionHandshake"), gAgent.mMOTD);
		if(gGotUseCircuitCodeAck)
		{
			LLStartUp::setStartupState( STATE_AGENT_SEND );
		}
		{
			LockMessageChecker lmc(gMessageSystem);
			while (lmc.checkAllMessages(gFrameCount, gServicePump))
			{
				display_startup();
			}
			lmc.processAcks();
		}
		display_startup();
		return FALSE;
	}

	//---------------------------------------------------------------------
	// Agent Send
	//---------------------------------------------------------------------
	if (STATE_AGENT_SEND == LLStartUp::getStartupState())
	{
		LL_DEBUGS("AppInit") << "Connecting to region..." << LL_ENDL;
		set_startup_status(0.60f, LLTrans::getString("LoginConnectingToRegion"), gAgent.mMOTD);
		display_startup();
		// register with the message system so it knows we're
		// expecting this message
		LLMessageSystem* msg = gMessageSystem;
		msg->setHandlerFuncFast(
			_PREHASH_AgentMovementComplete,
			process_agent_movement_complete);
		LLViewerRegion* regionp = gAgent.getRegion();
		if(regionp)
		{
			send_complete_agent_movement(regionp->getHost());
			gAssetStorage->setUpstream(regionp->getHost());
			gCacheName->setUpstream(regionp->getHost());
			// <FS:Ansariel> OpenSim legacy economy support
#ifdef OPENSIM
			if (!LLGridManager::instance().isInSecondLife())
			{
				msg->newMessageFast(_PREHASH_EconomyDataRequest);
				gAgent.sendReliableMessage();
			}
#endif
			// </FS:Ansariel>
		}
		display_startup();

		// Create login effect
		// But not on first login, because you can't see your avatar then
		if (!gAgent.isFirstLogin())
		{
			LLHUDEffectSpiral *effectp = (LLHUDEffectSpiral *)LLHUDManager::getInstance()->createViewerEffect(LLHUDObject::LL_HUD_EFFECT_POINT, TRUE);
			effectp->setPositionGlobal(gAgent.getPositionGlobal());
			effectp->setColor(LLColor4U(gAgent.getEffectColor()));
			LLHUDManager::getInstance()->sendEffects();
		}

		LLStartUp::setStartupState( STATE_AGENT_WAIT );		// Go to STATE_AGENT_WAIT

		timeout.reset();
		display_startup();
		return FALSE;
	}

	//---------------------------------------------------------------------
	// Agent Wait
	//---------------------------------------------------------------------
	if (STATE_AGENT_WAIT == LLStartUp::getStartupState())
	{
		{
			LockMessageChecker lmc(gMessageSystem);
			while (lmc.checkAllMessages(gFrameCount, gServicePump))
			{
				if (gAgentMovementCompleted)
				{
					// Sometimes we have more than one message in the
					// queue. break out of this loop and continue
					// processing. If we don't, then this could skip one
					// or more login steps.
					break;
				}
				else
				{
					LL_DEBUGS("AppInit") << "Awaiting AvatarInitComplete, got "
										 << gMessageSystem->getMessageName() << LL_ENDL;
				}
				display_startup();
			}
			lmc.processAcks();
		}

		display_startup();

		if (gAgentMovementCompleted)
		{
			LLStartUp::setStartupState( STATE_INVENTORY_SEND );
		}
		display_startup();

		if (!gAgentMovementCompleted && timeout.getElapsedTimeF32() > STATE_AGENT_WAIT_TIMEOUT)
		{
			LL_WARNS("AppInit") << "Backing up to login screen!" << LL_ENDL;
			if (gRememberPassword)
			{
				LLNotificationsUtil::add("LoginPacketNeverReceived", LLSD(), LLSD(), login_alert_status);
			}
			else
			{
				LLNotificationsUtil::add("LoginPacketNeverReceivedNoTP", LLSD(), LLSD(), login_alert_status);
			}
			reset_login();
		}
		return FALSE;
	}

	//---------------------------------------------------------------------
	// Inventory Send
	//---------------------------------------------------------------------
	if (STATE_INVENTORY_SEND == LLStartUp::getStartupState())
	{
		display_startup();

        // request mute list
        LL_INFOS() << "Requesting Mute List" << LL_ENDL;
        LLMuteList::getInstance()->requestFromServer(gAgent.getID());

        // Get L$ and ownership credit information
        LL_INFOS() << "Requesting Money Balance" << LL_ENDL;
        LLStatusBar::sendMoneyBalanceRequest();

        display_startup();
		// <FS:Ansariel> Moved before inventory creation.
		// request all group information
		LL_INFOS("Agent_GroupData") << "GROUPDEBUG: Requesting Agent Data during startup" << LL_ENDL;
		gAgent.sendAgentDataUpdateRequest();
		display_startup();
		// </FS:Ansariel>
		// Inform simulator of our language preference
		LLAgentLanguage::update();

		display_startup();
		// unpack thin inventory
		LLSD response = LLLoginInstance::getInstance()->getResponse();
		//bool dump_buffer = false;

		LLSD inv_lib_root = response["inventory-lib-root"];
		if(inv_lib_root.isDefined())
		{
			// should only be one
			LLSD id = inv_lib_root[0]["folder_id"];
			if(id.isDefined())
			{
				gInventory.setLibraryRootFolderID(id.asUUID());
			}
		}
		display_startup();
 		
		LLSD inv_lib_owner = response["inventory-lib-owner"];
		if(inv_lib_owner.isDefined())
		{
			// should only be one
			LLSD id = inv_lib_owner[0]["agent_id"];
			if(id.isDefined())
			{
				gInventory.setLibraryOwnerID( LLUUID(id.asUUID()));
			}
		}
		display_startup();

		LLSD inv_skel_lib = response["inventory-skel-lib"];
 		if(inv_skel_lib.isDefined() && gInventory.getLibraryOwnerID().notNull())
 		{
 			if(!gInventory.loadSkeleton(inv_skel_lib, gInventory.getLibraryOwnerID()))
 			{
 				LL_WARNS("AppInit") << "Problem loading inventory-skel-lib" << LL_ENDL;
 			}
 		}
		display_startup();

		LLSD inv_skeleton = response["inventory-skeleton"];
 		if(inv_skeleton.isDefined())
 		{
 			if(!gInventory.loadSkeleton(inv_skeleton, gAgent.getID()))
 			{
 				LL_WARNS("AppInit") << "Problem loading inventory-skel-targets" << LL_ENDL;
 			}
 		}
		display_startup();

		LLSD inv_basic = response["inventory-basic"];
 		if(inv_basic.isDefined())
 		{
			LL_INFOS() << "Basic inventory root folder id is " << inv_basic["folder_id"] << LL_ENDL;
 		}

		LLSD buddy_list = response["buddy-list"];
 		if(buddy_list.isDefined())
 		{
			LLAvatarTracker::buddy_map_t list;
			// <FS:Ansariel> Reset on each iteration
			//LLUUID agent_id;
			//S32 has_rights = 0, given_rights = 0;
			// </FS:Ansariel>
			for(LLSD::array_const_iterator it = buddy_list.beginArray(),
				end = buddy_list.endArray(); it != end; ++it)
			{
				// <FS:Ansariel> Reset on each iteration
				LLUUID agent_id;
				S32 has_rights = 0, given_rights = 0;
				// </FS:Ansariel>

				LLSD buddy_id = (*it)["buddy_id"];
				if(buddy_id.isDefined())
				{
					agent_id = buddy_id.asUUID();
				}
				// <FS:Ansariel> Can't add "friends" that have no ID
				else continue;

				LLSD buddy_rights_has = (*it)["buddy_rights_has"];
				if(buddy_rights_has.isDefined())
				{
					has_rights = buddy_rights_has.asInteger();
				}

				LLSD buddy_rights_given = (*it)["buddy_rights_given"];
				if(buddy_rights_given.isDefined())
				{
					given_rights = buddy_rights_given.asInteger();
				}

				list[agent_id] = new LLRelationship(given_rights, has_rights, false);
			}
			LLAvatarTracker::instance().addBuddyList(list);
			display_startup();
 		}

		// <FS:Ansariel> Contact sets
		LGGContactSets* cs_instance = LGGContactSets::getInstance();
		cs_instance->loadFromDisk();
		LLAvatarNameCache::instance().setCustomNameCheckCallback(boost::bind(&LGGContactSets::checkCustomName, cs_instance, _1, _2, _3));
		// </FS:Ansariel>

		bool show_hud = false;
		LLSD tutorial_setting = response["tutorial_setting"];
		if(tutorial_setting.isDefined())
		{
			for(LLSD::array_const_iterator it = tutorial_setting.beginArray(),
				end = tutorial_setting.endArray(); it != end; ++it)
			{
				LLSD tutorial_url = (*it)["tutorial_url"];
				if(tutorial_url.isDefined())
				{
					// Tutorial floater will append language code
					gSavedSettings.setString("TutorialURL", tutorial_url.asString());
				}
				
				// For Viewer 2.0 we are not using the web-based tutorial
				// If we reverse that decision, put this code back and use
				// login.cgi to send a different URL with content that matches
				// the Viewer 2.0 UI.
				//LLSD use_tutorial = (*it)["use_tutorial"];
				//if(use_tutorial.asString() == "true")
				//{
				//	show_hud = true;
				//}
			}
		}
		display_startup();

		// Either we want to show tutorial because this is the first login
		// to a Linden Help Island or the user quit with the tutorial
		// visible.  JC
		if (show_hud || gSavedSettings.getBOOL("ShowTutorial"))
		{
			LLFloaterReg::showInstance("hud", LLSD(), FALSE);
		}
		display_startup();

		LLSD event_notifications = response["event_notifications"];
		if(event_notifications.isDefined())
		{
			gEventNotifier.load(event_notifications);
		}
		display_startup();

		LLSD classified_categories = response["classified_categories"];
		if(classified_categories.isDefined())
		{
			LLClassifiedInfo::loadCategories(classified_categories);
		}
		display_startup();

		// This method MUST be called before gInventory.findCategoryUUIDForType because of 
		// gInventory.mIsAgentInvUsable is set to true in the gInventory.buildParentChildMap.
		gInventory.buildParentChildMap();
		gInventory.createCommonSystemCategories();

		// It's debatable whether this flag is a good idea - sets all
		// bits, and in general it isn't true that inventory
		// initialization generates all types of changes. Maybe add an
		// INITIALIZE mask bit instead?
		gInventory.addChangedMask(LLInventoryObserver::ALL, LLUUID::null);
		gInventory.notifyObservers();
		
		display_startup();

		//all categories loaded. lets create "My Favorites" category
		gInventory.findCategoryUUIDForType(LLFolderType::FT_FAVORITE,true);

		// set up callbacks
		LL_INFOS() << "Registering Callbacks" << LL_ENDL;
		LLMessageSystem* msg = gMessageSystem;
		LL_INFOS() << " Inventory" << LL_ENDL;
		LLInventoryModel::registerCallbacks(msg);
		LL_INFOS() << " AvatarTracker" << LL_ENDL;
		LLAvatarTracker::instance().registerCallbacks(msg);
		LL_INFOS() << " Landmark" << LL_ENDL;
		LLLandmark::registerCallbacks(msg);
		display_startup();

		// <FS:Ansariel> Moved before inventory creation.
		//// request all group information
		//LL_INFOS() << "Requesting Agent Data" << LL_ENDL;
		//gAgent.sendAgentDataUpdateRequest();
		//display_startup();
		// </FS:Ansariel>
		// Create the inventory views
		LL_INFOS() << "Creating Inventory Views" << LL_ENDL;
		LLFloaterReg::getInstance("inventory");
		//display_startup();

// [RLVa:KB] - Checked: RLVa-1.1.0
		if (RlvHandler::isEnabled())
		{
			// Regularly process a select subset of retained commands during logon
			gIdleCallbacks.addFunction(RlvHandler::onIdleStartup, new LLTimer());
		}
// [/RLVa:KB]

		LLStartUp::setStartupState( STATE_MISC );
		display_startup();
		return FALSE;
	}


	//---------------------------------------------------------------------
	// Misc
	//---------------------------------------------------------------------
	if (STATE_MISC == LLStartUp::getStartupState())
	{
		// We have a region, and just did a big inventory download.
		// We can estimate the user's connection speed, and set their
		// max bandwidth accordingly.  JC
		if (gSavedSettings.getBOOL("FirstLoginThisInstall"))
		{
			// This is actually a pessimistic computation, because TCP may not have enough
			// time to ramp up on the (small) default inventory file to truly measure max
			// bandwidth. JC
			F64 rate_bps = LLLoginInstance::getInstance()->getLastTransferRateBPS();
			const F32 FAST_RATE_BPS = 600.f * 1024.f;
			const F32 FASTER_RATE_BPS = 750.f * 1024.f;
			F32 max_bandwidth = gViewerThrottle.getMaxBandwidth();
			if (rate_bps > FASTER_RATE_BPS
				&& rate_bps > max_bandwidth)
			{
				LL_DEBUGS("AppInit") << "Fast network connection, increasing max bandwidth to " 
					<< FASTER_RATE_BPS/1024.f 
					<< " kbps" << LL_ENDL;
				gViewerThrottle.setMaxBandwidth(FASTER_RATE_BPS / 1024.f);
			}
			else if (rate_bps > FAST_RATE_BPS
				&& rate_bps > max_bandwidth)
			{
				LL_DEBUGS("AppInit") << "Fast network connection, increasing max bandwidth to " 
					<< FAST_RATE_BPS/1024.f 
					<< " kbps" << LL_ENDL;
				gViewerThrottle.setMaxBandwidth(FAST_RATE_BPS / 1024.f);
			}

			if (gSavedSettings.getBOOL("ShowHelpOnFirstLogin"))
			{
				gSavedSettings.setBOOL("HelpFloaterOpen", TRUE);
			}

			// Set the show start location to true, now that the user has logged
			// on with this install.
			gSavedSettings.setBOOL("ShowStartLocation", TRUE);

			// <FS:Ansariel> [FS Communication UI]
			LLFloaterReg::toggleInstanceOrBringToFront("fs_im_container");
			// </FS:Ansariel> [FS Communication UI]

		}
		
		display_startup();

        // Load stored local environment if needed.
        LLEnvironment::instance().loadFromSettings();

        // *TODO : Uncomment that line once the whole grid migrated to SLM and suppress it from LLAgent::handleTeleportFinished() (llagent.cpp)
        //check_merchant_status();

		display_startup();

		// <FS:CR> Compatibility with old backups
		// Put gSavedPerAccountSettings here, put gSavedSettings in llappviewer.cpp
		// *TODO: Should we keep these around forever or just three release cycles?
		if (gSavedSettings.getBOOL("FSFirstRunAfterSettingsRestore"))
		{
			// Post-chui merge logging change
			if (gSavedPerAccountSettings.getBOOL("LogInstantMessages"))
				gSavedPerAccountSettings.setS32("KeepConversationLogTranscript", 2);
			else
				gSavedPerAccountSettings.setS32("KeepConversationLogTranscript", 0);
			
			//ok, we're done, set it back to false.
			gSavedSettings.setBOOL("FSFirstRunAfterSettingsRestore", FALSE);
		}
		display_startup();
		// </FS:CR>

		if (gSavedSettings.getBOOL("HelpFloaterOpen"))
		{
			// show default topic
			LLViewerHelp::instance().showTopic("");
		}

		display_startup();

		// We're successfully logged in.
		gSavedSettings.setBOOL("FirstLoginThisInstall", FALSE);

		LLFloaterReg::showInitialVisibleInstances();

		LLFloaterGridStatus::getInstance()->startGridStatusTimer();

		display_startup();

		// <FS:Ansariel> [FS Login Panel]
		// based on the comments, we've successfully logged in so we can delete the 'forced'
		// URL that the updater set in settings.ini (in a mostly paranoid fashion)
		std::string nextLoginLocation = gSavedSettings.getString( "NextLoginLocation" );
		if ( nextLoginLocation.length() )
		{
			// clear it
			gSavedSettings.setString( "NextLoginLocation", "" );

			// and make sure it's saved
			gSavedSettings.saveToFile( gSavedSettings.getString("ClientSettingsFile") , TRUE );
			LLUIColorTable::instance().saveUserSettings();
		};
		// </FS:Ansariel> [FS Login Panel]

		display_startup();
		// JC: Initializing audio requests many sounds for download.
		init_audio();
		display_startup();

		// JC: Initialize "active" gestures.  This may also trigger
		// many gesture downloads, if this is the user's first
		// time on this machine or -purge has been run.
		LLSD gesture_options 
			= LLLoginInstance::getInstance()->getResponse("gestures");
		if (gesture_options.isDefined())
		{
			LL_DEBUGS("AppInit") << "Gesture Manager loading " << gesture_options.size()
				<< LL_ENDL;
			uuid_vec_t item_ids;
			for(LLSD::array_const_iterator resp_it = gesture_options.beginArray(),
				end = gesture_options.endArray(); resp_it != end; ++resp_it)
			{
				// If the id is not specifed in the LLSD,
				// the LLSD operator[]() will return a null LLUUID. 
				LLUUID item_id = (*resp_it)["item_id"];
				LLUUID asset_id = (*resp_it)["asset_id"];

				if (item_id.notNull() && asset_id.notNull())
				{
					// Could schedule and delay these for later.
					const BOOL no_inform_server = FALSE;
					const BOOL no_deactivate_similar = FALSE;
					LLGestureMgr::instance().activateGestureWithAsset(item_id, asset_id,
										 no_inform_server,
										 no_deactivate_similar);
					// We need to fetch the inventory items for these gestures
					// so we have the names to populate the UI.
					item_ids.push_back(item_id);
				}
			}
			// no need to add gesture to inventory observer, it's already made in constructor 
			LLGestureMgr::instance().setFetchIDs(item_ids);
			LLGestureMgr::instance().startFetch();
		}
		gDisplaySwapBuffers = TRUE;
		display_startup();

		LLMessageSystem* msg = gMessageSystem;
		msg->setHandlerFuncFast(_PREHASH_SoundTrigger,				process_sound_trigger);
		msg->setHandlerFuncFast(_PREHASH_PreloadSound,				process_preload_sound);
		msg->setHandlerFuncFast(_PREHASH_AttachedSound,				process_attached_sound);
		msg->setHandlerFuncFast(_PREHASH_AttachedSoundGainChange,	process_attached_sound_gain_change);

		LL_DEBUGS("AppInit") << "Initialization complete" << LL_ENDL;

		LL_DEBUGS("SceneLoadTiming", "Start") << "Scene Load Started " << LL_ENDL;
		gRenderStartTime.reset();
		gForegroundTime.reset();

		// HACK: Inform simulator of window size.
		// Do this here so it's less likely to race with RegisterNewAgent.
		// TODO: Put this into RegisterNewAgent
		// JC - 7/20/2002
		gViewerWindow->sendShapeToSim();

		LLPresetsManager::getInstance()->createMissingDefault(PRESETS_CAMERA);

		// The reason we show the alert is because we want to
		// reduce confusion for when you log in and your provided
		// location is not your expected location. So, if this is
		// your first login, then you do not have an expectation,
		// thus, do not show this alert.
		if (!gAgent.isFirstLogin())
		{
			LL_INFOS() << "gAgentStartLocation : " << gAgentStartLocation << LL_ENDL;
			LLSLURL start_slurl = LLStartUp::getStartSLURL();
			LL_DEBUGS("AppInit") << "start slurl "<<start_slurl.asString()<<LL_ENDL;
			
			if (((start_slurl.getType() == LLSLURL::LOCATION) && (gAgentStartLocation == "url")) ||
				((start_slurl.getType() == LLSLURL::LAST_LOCATION) && (gAgentStartLocation == "last")) ||
				((start_slurl.getType() == LLSLURL::HOME_LOCATION) && (gAgentStartLocation == "home")))
			{
				if (start_slurl.getType() == LLSLURL::LAST_LOCATION 
					&& gAgentStartLocation == "last" 
					&& gSavedSettings.getBOOL("RestoreCameraPosOnLogin"))
				{
					// restore old camera pos
					gAgentCamera.setFocusOnAvatar(FALSE, FALSE);
					gAgentCamera.setCameraPosAndFocusGlobal(gSavedSettings.getVector3d("CameraPosOnLogout"), gSavedSettings.getVector3d("FocusPosOnLogout"), LLUUID::null);
					BOOL limit_hit = FALSE;
					gAgentCamera.calcCameraPositionTargetGlobal(&limit_hit);
					if (limit_hit)
					{
						gAgentCamera.setFocusOnAvatar(TRUE, FALSE);
					}
					gAgentCamera.stopCameraAnimation();
				}
			}
			else
			{
				std::string msg;
				switch(start_slurl.getType())
				{
					case LLSLURL::LOCATION:
					{
						
						msg = "AvatarMovedDesired";
						break;
					}
					case LLSLURL::HOME_LOCATION:
					{
						msg = "AvatarMovedHome";
						break;
					}
					default:
					{
						msg = "AvatarMovedLast";
					}
				}
				LLNotificationsUtil::add(msg);
			}
		}

		display_startup();
        //DEV-17797.  get null folder.  Any items found here moved to Lost and Found
        LLInventoryModelBackgroundFetch::instance().findLostItems();
		display_startup();
		
		// <FS:CR> Load dynamic script library from xml
#ifdef OPENSIM
		if (LLGridManager::getInstance()->isInOpenSim())
		{
			if (!gScriptLibrary.loadLibrary(gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "scriptlibrary_ossl.xml")))
			{
				gScriptLibrary.loadLibrary(gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "scriptlibrary_ossl.xml"));
			}
		}
		if (LLGridManager::getInstance()->isInAuroraSim())
		{
			if (!gScriptLibrary.loadLibrary(gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "scriptlibrary_aa.xml")))
			{
				gScriptLibrary.loadLibrary(gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "scriptlibrary_aa.xml"));
			}
		}
#endif // OPENSIM
		display_startup();
		// </FS:CR>

		LLStartUp::setStartupState( STATE_PRECACHE );
		timeout.reset();
		return FALSE;
	}

	if (STATE_PRECACHE == LLStartUp::getStartupState())
	{
		display_startup();
		F32 timeout_frac = timeout.getElapsedTimeF32()/PRECACHING_DELAY;
		
		// We now have an inventory skeleton, so if this is a user's first
		// login, we can start setting up their clothing and avatar 
		// appearance.  This helps to avoid the generic "Ruth" avatar in
		// the orientation island tutorial experience. JC
		if (gAgent.isFirstLogin()
			&& !sInitialOutfit.empty()    // registration set up an outfit
			&& !sInitialOutfitGender.empty() // and a gender
			&& isAgentAvatarValid()	  // can't wear clothes without object
			&& !gAgent.isOutfitChosen()) // nothing already loading
		{
			// Start loading the wearables, textures, gestures
			LLStartUp::loadInitialOutfit( sInitialOutfit, sInitialOutfitGender );
		}
		// If not first login, we need to fetch COF contents and
		// compute appearance from that.
		if (isAgentAvatarValid() && !gAgent.isFirstLogin() && !gAgent.isOutfitChosen())
		{
			gAgentWearables.notifyLoadingStarted();
			gAgent.setOutfitChosen(TRUE);
// <FS:Ansariel> [Legacy Bake]
			//gAgentWearables.sendDummyAgentWearablesUpdate();
			if (LLGridManager::getInstance()->isInSecondLife())
			{
				gAgentWearables.sendDummyAgentWearablesUpdate();
			}
// </FS:Ansariel> [Legacy Bake]
			callAfterCategoryFetch(LLAppearanceMgr::instance().getCOF(), set_flags_and_update_appearance);
		}

		display_startup();

		// wait precache-delay and for agent's avatar or a lot longer.
		if ((timeout_frac > 1.f) && isAgentAvatarValid())
		{
			LLStartUp::setStartupState( STATE_WEARABLES_WAIT );
		}
		else if (timeout_frac > 10.f) 
		{
			// If we exceed the wait above while isAgentAvatarValid is
			// not true yet, we will change startup state and
			// eventually (once avatar does get created) wind up at
			// the gender chooser. This should occur only in very
			// unusual circumstances, so set the timeout fairly high
			// to minimize mistaken hits here.
			LL_WARNS() << "Wait for valid avatar state exceeded " 
					<< timeout.getElapsedTimeF32() << " will invoke gender chooser" << LL_ENDL; 
			LLStartUp::setStartupState( STATE_WEARABLES_WAIT );
		}
		else
		{
			update_texture_fetch();
			set_startup_status(0.60f + 0.30f * timeout_frac,
				LLTrans::getString("LoginPrecaching"),
					gAgent.mMOTD.c_str());
			display_startup();
		}
		
		return TRUE;
	}

	if (STATE_WEARABLES_WAIT == LLStartUp::getStartupState())
	{
		static LLFrameTimer wearables_timer;

		const F32 wearables_time = wearables_timer.getElapsedTimeF32();
		static LLCachedControl<F32> max_wearables_time(gSavedSettings, "ClothingLoadingDelay");

		if (!gAgent.isOutfitChosen() && isAgentAvatarValid())
		{
			// No point in waiting for clothing, we don't even know
			// what outfit we want.  Pop up a gender chooser dialog to
			// ask and proceed to draw the world. JC
			//
			// *NOTE: We might hit this case even if we have an
			// initial outfit, but if the load hasn't started
			// already then something is wrong so fall back
			// to generic outfits. JC
			LLNotificationsUtil::add("WelcomeChooseSex", LLSD(), LLSD(),
				callback_choose_gender);
			LLStartUp::setStartupState( STATE_CLEANUP );
		}
		
		display_startup();

		if (gAgent.isOutfitChosen() && (wearables_time > max_wearables_time))
		{
			LLNotificationsUtil::add("ClothingLoading");
			record(LLStatViewer::LOADING_WEARABLES_LONG_DELAY, wearables_time);
			LLStartUp::setStartupState( STATE_CLEANUP );
		}
		else if (gAgent.isFirstLogin()
				&& isAgentAvatarValid()
				&& gAgentAvatarp->isFullyLoaded())
		{
			// wait for avatar to be completely loaded
			if (isAgentAvatarValid()
				&& gAgentAvatarp->isFullyLoaded())
			{
				LL_DEBUGS("Avatar") << "avatar fully loaded" << LL_ENDL;
				LLStartUp::setStartupState( STATE_CLEANUP );
				return TRUE;
			}
		}
		else
		{
			// OK to just get the wearables
			if ( gAgentWearables.areWearablesLoaded() )
			{
				// We have our clothing, proceed.
				LL_DEBUGS("Avatar") << "wearables loaded" << LL_ENDL;
				LLStartUp::setStartupState( STATE_CLEANUP );
				return TRUE;
			}

			// <FS:Ansariel> Can't fall through here, so return
			return TRUE;
		}
		//fall through this frame to STATE_CLEANUP
	}

	if (STATE_CLEANUP == LLStartUp::getStartupState())
	{
		set_startup_status(1.0, "", "");
		display_startup();

		if (!mBenefitsSuccessfullyInit)
		{
			LLNotificationsUtil::add("FailedToGetBenefits", LLSD(), LLSD(), boost::bind(on_benefits_failed_callback, _1, _2));
		}

		// <FS:TT> Client LSL Bridge
		if (gSavedSettings.getBOOL("UseLSLBridge"))
		{
			if (!FSLSLBridge::instance().getBridgeCreating())
			{
				FSLSLBridge::instance().initBridge();
			}
			else
			{
				LL_INFOS("FSLSLBridge") << "LSL bridge already getting created - skipping bridge init" << LL_ENDL;
			}
		}
		// </FS:TT>

		// <FS:Ansariel> Favorite Wearables
		FSFloaterWearableFavorites::initCategory();

		// <FS:Ansariel> Bypass the calling card sync-crap to create the agent's calling card
		LLFriendCardsManager::createAgentCallingCard();

		// Let the map know about the inventory.
		LLFloaterWorldMap* floater_world_map = LLFloaterWorldMap::getInstance();
		if(floater_world_map)
		{
			floater_world_map->observeInventory(&gInventory);
			floater_world_map->observeFriends();
		}
		gViewerWindow->showCursor();
		gViewerWindow->getWindow()->resetBusyCount();
		gViewerWindow->getWindow()->setCursor(UI_CURSOR_ARROW);
		LL_DEBUGS("AppInit") << "Done releasing bitmap" << LL_ENDL;
		//gViewerWindow->revealIntroPanel();
		gViewerWindow->setStartupComplete(); 
		gViewerWindow->setProgressCancelButtonVisible(FALSE);
		display_startup();

		// We're not away from keyboard, even though login might have taken
		// a while. JC
		gAgent.clearAFK();

		// Have the agent start watching the friends list so we can update proxies
		gAgent.observeFriends();
		if (gSavedSettings.getBOOL("LoginAsGod"))
		{
			gAgent.requestEnterGodMode();
		}
		
		// Start automatic replay if the flag is set.
		if (gSavedSettings.getBOOL("StatsAutoRun") || gAgentPilot.getReplaySession())
		{
			LL_DEBUGS("AppInit") << "Starting automatic playback" << LL_ENDL;
			gAgentPilot.startPlayback();
		}

		show_debug_menus(); // Debug menu visiblity and First Use trigger
		
		// If we've got a startup URL, dispatch it
		//LLStartUp::dispatchURL();

		// Retrieve information about the land data
		// (just accessing this the first time will fetch it,
		// then the data is cached for the viewer's lifetime)
		LLProductInfoRequestManager::instance();
		
		// *FIX:Mani - What do I do here?
		// Need we really clear the Auth response data?
		// Clean up the userauth stuff.
		// LLUserAuth::getInstance()->reset();

		LLStartUp::setStartupState( STATE_STARTED );
		display_startup();

		// <FS:Ansariel> Draw Distance stepping; originally based on SpeedRez by Henri Beauchamp, licensed under LGPL
		if (gSavedSettings.getBOOL("FSRenderFarClipStepping"))
		{
			// progressive draw distance stepping if requested.
			LLPresetsManager::instance().setIsDrawDistanceSteppingActive(true);
			F32 dist1 = gSavedSettings.getF32("RenderFarClip");
			F32 dist2 = gSavedSettings.getF32("FSSavedRenderFarClip");
			gSavedDrawDistance = (dist1 >= dist2 ? dist1 : dist2);
			gSavedSettings.setF32("FSSavedRenderFarClip", gSavedDrawDistance);
			gSavedSettings.setF32("RenderFarClip", 32.0f);
			gLastDrawDistanceStep = 32.0f;
		}
		// </FS:Ansariel>

		// Unmute audio if desired and setup volumes.
		// This is a not-uncommon crash site, so surround it with
		// LL_INFOS() output to aid diagnosis.
		LL_INFOS("AppInit") << "Doing first audio_update_volume..." << LL_ENDL;
		audio_update_volume();
		LL_INFOS("AppInit") << "Done first audio_update_volume." << LL_ENDL;

		// reset keyboard focus to sane state of pointing at world
		gFocusMgr.setKeyboardFocus(NULL);

		LLAppViewer::instance()->handleLoginComplete();

		LLAgentPicksInfo::getInstance()->requestNumberOfPicks();

		// <FS:Ansariel> [FS communication UI]
		FSFloaterIM::initIMFloater();
		// </FS:Ansariel> [FS communication UI]
		display_startup();

		llassert(LLPathfindingManager::getInstance() != NULL);
		LLPathfindingManager::getInstance()->initSystem();

		gAgentAvatarp->sendHoverHeight();

		// look for parcels we own
		send_places_query(LLUUID::null,
			LLUUID::null,
			"",
			DFQ_AGENT_OWNED,
			LLParcel::C_ANY,
			"");

		LLUIUsage::instance().clear();

		// <FS:Techwolf Lupindo> FIRE-6643 Display MOTD when login screens are disabled
		if (gSavedSettings.getBOOL("FSDisableLoginScreens"))
		{
			report_to_nearby_chat(gAgent.mMOTD);
		}
		// </FS:Techwolf Lupindo>
		// <FS:PP>
		if (gSavedSettings.getBOOL("AutoQueryGridStatus"))
		{
			FSCoreHttpUtil::callbackHttpGetRaw(gSavedSettings.getString("AutoQueryGridStatusURL"),
				downloadGridstatusComplete, [](const LLSD& data) { downloadGridstatusError(data, gSavedSettings.getString("AutoQueryGridStatusURL")); });
		}
		// </FS:PP>

		return TRUE;
	}

	return TRUE;
}

//
// local function definition
//

void login_show()
{
	LL_INFOS("AppInit") << "Initializing Login Screen" << LL_ENDL;

	// Hide the toolbars: may happen to come back here if login fails after login agent but before login in region
	if (gToolBarView)
	{
		gToolBarView->setVisible(FALSE);
	}
	
	// <FS:Ansariel> [FS Login Panel]
	//LLPanelLogin::show(	gViewerWindow->getWindowRectScaled(), login_callback, NULL );
	FSPanelLogin::show(	gViewerWindow->getWindowRectScaled(), login_callback, NULL );
	// </FS:Ansariel> [FS Login Panel]
}

// Callback for when login screen is closed.  Option 0 = connect, option 1 = quit.
void login_callback(S32 option, void *userdata)
{
	const S32 CONNECT_OPTION = 0;
	const S32 QUIT_OPTION = 1;

	if (CONNECT_OPTION == option)
	{
		LLStartUp::setStartupState( STATE_LOGIN_CLEANUP );
		return;
	}
	else if (QUIT_OPTION == option) // *TODO: THIS CODE SEEMS TO BE UNREACHABLE!!!!! login_callback is never called with option equal to QUIT_OPTION
	{
		if (!gSavedSettings.getBOOL("RememberPassword"))
		{
			// turn off the setting and write out to disk
			gSavedSettings.saveToFile( gSavedSettings.getString("ClientSettingsFile") , TRUE );
			LLUIColorTable::instance().saveUserSettings();
		}

		// Next iteration through main loop should shut down the app cleanly.
		LLAppViewer::instance()->userQuit();
		
		if (LLAppViewer::instance()->quitRequested())
		{
			// <FS:Ansariel> [FS Login Panel]
			//LLPanelLogin::closePanel();
			FSPanelLogin::closePanel();
			// </FS:Ansariel> [FS Login Panel]
		}
		return;
	}
	else
	{
		LL_WARNS("AppInit") << "Unknown login button clicked" << LL_ENDL;
	}
}

/**
* Check if user is running a new version of the viewer.
* Display the Release Notes if it's not overriden by the "UpdaterShowReleaseNotes" setting.
*/
void show_release_notes_if_required()
{
    static bool release_notes_shown = false;
    if (!release_notes_shown && (LLVersionInfo::instance().getChannelAndVersion() != gLastRunVersion)
        && LLVersionInfo::instance().getViewerMaturity() != LLVersionInfo::TEST_VIEWER // don't show Release Notes for the test builds
        && gSavedSettings.getBOOL("UpdaterShowReleaseNotes")
        && !gSavedSettings.getBOOL("FirstLoginThisInstall"))
    {
        LLSD info(LLAppViewer::instance()->getViewerInfo());
        LLWeb::loadURLInternal(info["VIEWER_RELEASE_NOTES_URL"]);
        release_notes_shown = true;
    }
}

// <FS:CR> Ditch the first run modal. Assume the user already has an account.
//void show_first_run_dialog()
//{
//	LLNotificationsUtil::add("FirstRun", LLSD(), LLSD(), first_run_dialog_callback);
//}

bool first_run_dialog_callback(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotificationsUtil::getSelectedOption(notification, response);
	if (0 == option)
	{
		LL_DEBUGS("AppInit") << "First run dialog cancelling" << LL_ENDL;
		LLWeb::loadURLExternal(LLTrans::getString("create_account_url") );
	}

	// <FS:Ansariel> [FS Login Panel]
	//LLPanelLogin::giveFocus();
	FSPanelLogin::giveFocus();
	// </FS:Ansariel> [FS Login Panel]
	return false;
}



void set_startup_status(const F32 frac, const std::string& string, const std::string& msg)
{
	gViewerWindow->setProgressPercent(frac*100);
	gViewerWindow->setProgressString(string);

	gViewerWindow->setProgressMessage(msg);
}

bool login_alert_status(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotificationsUtil::getSelectedOption(notification, response);
    // Buttons
    switch( option )
    {
        case 0:     // OK
            break;
      //  case 1:     // Help
      //      LLWeb::loadURL(LLNotifications::instance().getGlobalString("SUPPORT_URL") );
      //      break;
        case 2:     // Teleport
            // Restart the login process, starting at our home locaton
	  LLStartUp::setStartSLURL(LLSLURL(LLSLURL::SIM_LOCATION_HOME));
            LLStartUp::setStartupState( STATE_LOGIN_CLEANUP );
            break;
        default:
            LL_WARNS("AppInit") << "Missing case in login_alert_status switch" << LL_ENDL;
    }

	// <FS:Ansariel> [FS Login Panel]
	//LLPanelLogin::giveFocus();
	FSPanelLogin::giveFocus();
	// </FS:Ansariel> [FS Login Panel]
	return false;
}


void use_circuit_callback(void**, S32 result)
{
	// bail if we're quitting.
	if(LLApp::isExiting()) return;
	if( !gUseCircuitCallbackCalled )
	{
		gUseCircuitCallbackCalled = true;
		if (result)
		{
			// Make sure user knows something bad happened. JC
			LL_WARNS("AppInit") << "Backing up to login screen!" << LL_ENDL;
			if (gRememberPassword)
			{
				LLNotificationsUtil::add("LoginPacketNeverReceived", LLSD(), LLSD(), login_alert_status);
			}
			else
			{
				LLNotificationsUtil::add("LoginPacketNeverReceivedNoTP", LLSD(), LLSD(), login_alert_status);
			}
			reset_login();
		}
		else
		{
			gGotUseCircuitCodeAck = true;
		}
	}
}

void register_viewer_callbacks(LLMessageSystem* msg)
{
	msg->setHandlerFuncFast(_PREHASH_LayerData,				process_layer_data );
	msg->setHandlerFuncFast(_PREHASH_ImageData,				LLViewerTextureList::receiveImageHeader );
	msg->setHandlerFuncFast(_PREHASH_ImagePacket,				LLViewerTextureList::receiveImagePacket );
	msg->setHandlerFuncFast(_PREHASH_ObjectUpdate,				process_object_update );
	msg->setHandlerFunc("ObjectUpdateCompressed",				process_compressed_object_update );
	msg->setHandlerFunc("ObjectUpdateCached",					process_cached_object_update );
	msg->setHandlerFuncFast(_PREHASH_ImprovedTerseObjectUpdate, process_terse_object_update_improved );
	msg->setHandlerFunc("SimStats",				process_sim_stats);
	msg->setHandlerFuncFast(_PREHASH_HealthMessage,			process_health_message );
	msg->setHandlerFuncFast(_PREHASH_EconomyData,				process_economy_data);
	msg->setHandlerFunc("RegionInfo", LLViewerRegion::processRegionInfo);

	msg->setHandlerFuncFast(_PREHASH_ChatFromSimulator,		process_chat_from_simulator);
	msg->setHandlerFuncFast(_PREHASH_KillObject,				process_kill_object,	NULL);
	msg->setHandlerFuncFast(_PREHASH_SimulatorViewerTimeMessage,	process_time_synch,		NULL);
	msg->setHandlerFuncFast(_PREHASH_EnableSimulator,			process_enable_simulator);
	msg->setHandlerFuncFast(_PREHASH_DisableSimulator,			process_disable_simulator);
	msg->setHandlerFuncFast(_PREHASH_KickUser,					process_kick_user,		NULL);

	msg->setHandlerFunc("CrossedRegion", process_crossed_region);
	msg->setHandlerFuncFast(_PREHASH_TeleportFinish, process_teleport_finish);

	msg->setHandlerFuncFast(_PREHASH_AlertMessage,             process_alert_message);
	msg->setHandlerFunc("AgentAlertMessage", process_agent_alert_message);
	msg->setHandlerFuncFast(_PREHASH_MeanCollisionAlert,             process_mean_collision_alert_message,  NULL);
	msg->setHandlerFunc("ViewerFrozenMessage",             process_frozen_message);

	msg->setHandlerFuncFast(_PREHASH_NameValuePair,			process_name_value);
	msg->setHandlerFuncFast(_PREHASH_RemoveNameValuePair,	process_remove_name_value);
	msg->setHandlerFuncFast(_PREHASH_AvatarAnimation,		process_avatar_animation);
	msg->setHandlerFuncFast(_PREHASH_ObjectAnimation,		process_object_animation);
	msg->setHandlerFuncFast(_PREHASH_AvatarAppearance,		process_avatar_appearance);
	// <FS:Ansariel> [Legacy Bake]
	msg->setHandlerFunc("AgentCachedTextureResponse",	LLAgent::processAgentCachedTextureResponse);
	msg->setHandlerFunc("RebakeAvatarTextures", LLVOAvatarSelf::processRebakeAvatarTextures);
	// </FS:Ansariel> [Legacy Bake]
	msg->setHandlerFuncFast(_PREHASH_CameraConstraint,		process_camera_constraint);
	msg->setHandlerFuncFast(_PREHASH_AvatarSitResponse,		process_avatar_sit_response);
	msg->setHandlerFunc("SetFollowCamProperties",			process_set_follow_cam_properties);
	msg->setHandlerFunc("ClearFollowCamProperties",			process_clear_follow_cam_properties);

	msg->setHandlerFuncFast(_PREHASH_ImprovedInstantMessage,	process_improved_im);
	msg->setHandlerFuncFast(_PREHASH_ScriptQuestion,			process_script_question);
	// <FS:Techwolf Lupindo> area search
	//msg->setHandlerFuncFast(_PREHASH_ObjectProperties,			LLSelectMgr::processObjectProperties, NULL);
	msg->setHandlerFuncFast(_PREHASH_ObjectProperties,			process_object_properties, NULL);
	// </FS:Techwolf Lupindo> area search
	// <FS:Ansariel> Anti spam
	//msg->setHandlerFuncFast(_PREHASH_ObjectPropertiesFamily,	LLSelectMgr::processObjectPropertiesFamily, NULL);
	msg->setHandlerFuncFast(_PREHASH_ObjectPropertiesFamily,	process_object_properties_family, NULL);
	// </FS:Ansariel>
	msg->setHandlerFunc("ForceObjectSelect", LLSelectMgr::processForceObjectSelect);

	msg->setHandlerFuncFast(_PREHASH_MoneyBalanceReply,		process_money_balance_reply,	NULL);
	msg->setHandlerFuncFast(_PREHASH_CoarseLocationUpdate,		LLWorld::processCoarseUpdate, NULL);
	msg->setHandlerFuncFast(_PREHASH_ReplyTaskInventory, 		LLViewerObject::processTaskInv,	NULL);
	msg->setHandlerFuncFast(_PREHASH_DerezContainer,			process_derez_container, NULL);
	msg->setHandlerFuncFast(_PREHASH_ScriptRunningReply,
						&LLLiveLSLEditor::processScriptRunningReply);

	msg->setHandlerFuncFast(_PREHASH_DeRezAck, process_derez_ack);

	msg->setHandlerFunc("LogoutReply", process_logout_reply);

	//msg->setHandlerFuncFast(_PREHASH_AddModifyAbility,
	//					&LLAgent::processAddModifyAbility);
	//msg->setHandlerFuncFast(_PREHASH_RemoveModifyAbility,
	//					&LLAgent::processRemoveModifyAbility);
	msg->setHandlerFuncFast(_PREHASH_AgentDataUpdate,
						&LLAgent::processAgentDataUpdate);
	msg->setHandlerFuncFast(_PREHASH_AgentGroupDataUpdate,
						&LLAgent::processAgentGroupDataUpdate);
	msg->setHandlerFunc("AgentDropGroup",
						&LLAgent::processAgentDropGroup);
	// land ownership messages
	msg->setHandlerFuncFast(_PREHASH_ParcelOverlay,
						LLViewerParcelMgr::processParcelOverlay);
	msg->setHandlerFuncFast(_PREHASH_ParcelProperties,
						LLViewerParcelMgr::processParcelProperties);
	msg->setHandlerFunc("ParcelAccessListReply",
		LLViewerParcelMgr::processParcelAccessListReply);
	msg->setHandlerFunc("ParcelDwellReply",
		LLViewerParcelMgr::processParcelDwellReply);

	msg->setHandlerFunc("AvatarPropertiesReply",
						&LLAvatarPropertiesProcessor::processAvatarPropertiesReply);
	msg->setHandlerFunc("AvatarInterestsReply",
						&LLAvatarPropertiesProcessor::processAvatarInterestsReply);
	msg->setHandlerFunc("AvatarGroupsReply",
						&LLAvatarPropertiesProcessor::processAvatarGroupsReply);
	// ratings deprecated
	//msg->setHandlerFuncFast(_PREHASH_AvatarStatisticsReply,
	//					LLPanelAvatar::processAvatarStatisticsReply);
	msg->setHandlerFunc("AvatarNotesReply",
						&LLAvatarPropertiesProcessor::processAvatarNotesReply);
	msg->setHandlerFunc("AvatarPicksReply",
						&LLAvatarPropertiesProcessor::processAvatarPicksReply);
 	msg->setHandlerFunc("AvatarClassifiedReply",
 						&LLAvatarPropertiesProcessor::processAvatarClassifiedsReply);

	msg->setHandlerFuncFast(_PREHASH_CreateGroupReply,
						LLGroupMgr::processCreateGroupReply);
	msg->setHandlerFuncFast(_PREHASH_JoinGroupReply,
						LLGroupMgr::processJoinGroupReply);
	msg->setHandlerFuncFast(_PREHASH_EjectGroupMemberReply,
						LLGroupMgr::processEjectGroupMemberReply);
	msg->setHandlerFuncFast(_PREHASH_LeaveGroupReply,
						LLGroupMgr::processLeaveGroupReply);
	msg->setHandlerFuncFast(_PREHASH_GroupProfileReply,
						LLGroupMgr::processGroupPropertiesReply);

	// ratings deprecated
	// msg->setHandlerFuncFast(_PREHASH_ReputationIndividualReply,
	//					LLFloaterRate::processReputationIndividualReply);

	// <FS:Ansariel> [Legacy Bake]
	msg->setHandlerFuncFast(_PREHASH_AgentWearablesUpdate, LLAgentWearables::processAgentInitialWearablesUpdate );

	msg->setHandlerFunc("ScriptControlChange",
						LLAgent::processScriptControlChange );

	msg->setHandlerFuncFast(_PREHASH_ViewerEffect, LLHUDManager::processViewerEffect);

	msg->setHandlerFuncFast(_PREHASH_GrantGodlikePowers, process_grant_godlike_powers);

	msg->setHandlerFuncFast(_PREHASH_GroupAccountSummaryReply,
							LLPanelGroupLandMoney::processGroupAccountSummaryReply);
	msg->setHandlerFuncFast(_PREHASH_GroupAccountDetailsReply,
							LLPanelGroupLandMoney::processGroupAccountDetailsReply);
	msg->setHandlerFuncFast(_PREHASH_GroupAccountTransactionsReply,
							LLPanelGroupLandMoney::processGroupAccountTransactionsReply);

	msg->setHandlerFuncFast(_PREHASH_UserInfoReply,
		process_user_info_reply);

	msg->setHandlerFunc("RegionHandshake", process_region_handshake, NULL);

	msg->setHandlerFunc("TeleportStart", process_teleport_start );
	msg->setHandlerFunc("TeleportProgress", process_teleport_progress);
	msg->setHandlerFunc("TeleportFailed", process_teleport_failed, NULL);
	msg->setHandlerFunc("TeleportLocal", process_teleport_local, NULL);

	msg->setHandlerFunc("ImageNotInDatabase", LLViewerTextureList::processImageNotInDatabase, NULL);

	msg->setHandlerFuncFast(_PREHASH_GroupMembersReply,
						LLGroupMgr::processGroupMembersReply);
	msg->setHandlerFunc("GroupRoleDataReply",
						LLGroupMgr::processGroupRoleDataReply);
	msg->setHandlerFunc("GroupRoleMembersReply",
						LLGroupMgr::processGroupRoleMembersReply);
	msg->setHandlerFunc("GroupTitlesReply",
						LLGroupMgr::processGroupTitlesReply);
	// Special handler as this message is sometimes used for group land.
	msg->setHandlerFunc("PlacesReply", process_places_reply);
	msg->setHandlerFunc("GroupNoticesListReply", LLPanelGroupNotices::processGroupNoticesListReply);

// <FS:CR> FIRE-6310 - Legacy search handlers
	msg->setHandlerFunc("DirPeopleReply", FSPanelSearchPeople::processSearchReply);
	msg->setHandlerFunc("DirPlacesReply", FSPanelSearchPlaces::processSearchReply);
	msg->setHandlerFunc("DirGroupsReply", FSPanelSearchGroups::processSearchReply);
	msg->setHandlerFunc("DirEventsReply", FSPanelSearchEvents::processSearchReply);
	msg->setHandlerFunc("DirLandReply",   FSPanelSearchLand::processSearchReply);
	msg->setHandlerFunc("DirClassifiedReply",  FSPanelSearchClassifieds::processSearchReply);
// </FS:CR> FIRE-6310
	msg->setHandlerFunc("AvatarPickerReply", LLFloaterAvatarPicker::processAvatarPickerReply);

	msg->setHandlerFunc("MapBlockReply", LLWorldMapMessage::processMapBlockReply);
	msg->setHandlerFunc("MapItemReply", LLWorldMapMessage::processMapItemReply);
	msg->setHandlerFunc("EventInfoReply", LLEventNotifier::processEventInfoReply);
	
	msg->setHandlerFunc("PickInfoReply", &LLAvatarPropertiesProcessor::processPickInfoReply);
//	msg->setHandlerFunc("ClassifiedInfoReply", LLPanelClassified::processClassifiedInfoReply);
	msg->setHandlerFunc("ClassifiedInfoReply", LLAvatarPropertiesProcessor::processClassifiedInfoReply);
	msg->setHandlerFunc("ParcelInfoReply", LLRemoteParcelInfoProcessor::processParcelInfoReply);
	msg->setHandlerFunc("ScriptDialog", process_script_dialog);
	msg->setHandlerFunc("LoadURL", process_load_url);
	msg->setHandlerFunc("ScriptTeleportRequest", process_script_teleport_request);
	msg->setHandlerFunc("EstateCovenantReply", process_covenant_reply);

	// calling cards
	msg->setHandlerFunc("OfferCallingCard", process_offer_callingcard);
	msg->setHandlerFunc("AcceptCallingCard", process_accept_callingcard);
	msg->setHandlerFunc("DeclineCallingCard", process_decline_callingcard);

	msg->setHandlerFunc("ParcelObjectOwnersReply", LLPanelLandObjects::processParcelObjectOwnersReply);

	msg->setHandlerFunc("InitiateDownload", process_initiate_download);
	msg->setHandlerFunc("LandStatReply", LLFloaterTopObjects::handle_land_reply);
    msg->setHandlerFunc("GenericMessage", process_generic_message);
    msg->setHandlerFunc("LargeGenericMessage", process_large_generic_message);

	msg->setHandlerFuncFast(_PREHASH_FeatureDisabled, process_feature_disabled_message);
}

void asset_callback_nothing(const LLUUID&, LLAssetType::EType, void*, S32)
{
	// nothing
}

const S32 OPT_CLOSED_WINDOW = -1;
const S32 OPT_MALE = 0;
const S32 OPT_FEMALE = 1;
const S32 OPT_TRUST_CERT = 0;
const S32 OPT_CANCEL_TRUST = 1;
	
bool callback_choose_gender(const LLSD& notification, const LLSD& response)
{
	
    // These defaults are returned from the server on login.  They are set in login.xml.                  
    // If no default is returned from the server, they are retrieved from settings.xml.                   
	
	S32 option = LLNotification::getSelectedOption(notification, response);
	switch(option)
	{
		case OPT_MALE:
			LLStartUp::loadInitialOutfit( gSavedSettings.getString("DefaultMaleAvatar"), "male" );
			break;
			
        case OPT_FEMALE:
        case OPT_CLOSED_WINDOW:
        default:
			LLStartUp::loadInitialOutfit( gSavedSettings.getString("DefaultFemaleAvatar"), "female" );
			break;
	}
	return false;
}

std::string get_screen_filename(const std::string& pattern)
{
    // <FS:Ansariel> OpenSim support
    //if (LLGridManager::getInstance()->isInProductionGrid())
    if (LLGridManager::getInstance()->isInSLMain())
    // </FS:Ansariel>
    {
        return llformat(pattern.c_str(), "");
    }
    else
    {
        const std::string& grid_id_str = LLGridManager::getInstance()->getGridId();
        const std::string& grid_id_lower = utf8str_tolower(grid_id_str);
        std::string grid = "." + grid_id_lower;
        return llformat(pattern.c_str(), grid.c_str());
    }
}

//static
std::string LLStartUp::getScreenLastFilename()
{
    return get_screen_filename(SCREEN_LAST_FILENAME);
}

//static
std::string LLStartUp::getScreenHomeFilename()
{
    return get_screen_filename(SCREEN_HOME_FILENAME);
}

//static
void LLStartUp::loadInitialOutfit( const std::string& outfit_folder_name,
								   const std::string& gender_name )
{
	LL_DEBUGS() << "starting" << LL_ENDL;

	// Not going through the processAgentInitialWearables path, so need to set this here.
	LLAppearanceMgr::instance().setAttachmentInvLinkEnable(true);
	// Initiate creation of COF, since we're also bypassing that.
	gInventory.findCategoryUUIDForType(LLFolderType::FT_CURRENT_OUTFIT);
	
	ESex gender;
	if (gender_name == "male")
	{
		LL_DEBUGS() << "male" << LL_ENDL;
		gender = SEX_MALE;
	}
	else
	{
		LL_DEBUGS() << "female" << LL_ENDL;
		gender = SEX_FEMALE;
	}

	if (!isAgentAvatarValid())
	{
		LL_WARNS() << "Trying to load an initial outfit for an invalid agent avatar" << LL_ENDL;
		return;
	}

	gAgentAvatarp->setSex(gender);

	// try to find the outfit - if not there, create some default
	// wearables.
	LLUUID cat_id = findDescendentCategoryIDByName(
		gInventory.getLibraryRootFolderID(),
		outfit_folder_name);
	if (cat_id.isNull())
	{
		LL_DEBUGS() << "standard wearables" << LL_ENDL;
		gAgentWearables.createStandardWearables();
	}
	else
	{
		bool do_copy = true;
		bool do_append = false;
		LLViewerInventoryCategory *cat = gInventory.getCategory(cat_id);
		// Need to fetch cof contents before we can wear.
		callAfterCategoryFetch(LLAppearanceMgr::instance().getCOF(),
							   boost::bind(&LLAppearanceMgr::wearInventoryCategory, LLAppearanceMgr::getInstance(), cat, do_copy, do_append));
		LL_DEBUGS() << "initial outfit category id: " << cat_id << LL_ENDL;
	}

	gAgent.setOutfitChosen(TRUE);
// <FS:Ansariel> [Legacy Bake]
#ifdef OPENSIM
	if (LLGridManager::getInstance()->isInSecondLife())
#endif
// </FS:Ansariel> [Legacy Bake]
	gAgentWearables.sendDummyAgentWearablesUpdate();
}

std::string& LLStartUp::getInitialOutfitName()
{
	return sInitialOutfit;
}

std::string LLStartUp::getUserId()
{
    if (gUserCredential.isNull())
    {
        return "";
    }
    return gUserCredential->userID();
}


// frees the bitmap
void release_start_screen()
{
	LL_DEBUGS("AppInit") << "Releasing bitmap..." << LL_ENDL;
	gStartTexture = NULL;
}


// static
std::string LLStartUp::startupStateToString(EStartupState state)
{
#define RTNENUM(E) case E: return #E
	switch(state){
		RTNENUM( STATE_FIRST );
		RTNENUM( STATE_BROWSER_INIT );
		RTNENUM( STATE_LOGIN_SHOW );
		RTNENUM( STATE_LOGIN_WAIT );
		RTNENUM( STATE_LOGIN_CLEANUP );
		RTNENUM( STATE_LOGIN_AUTH_INIT );
		RTNENUM( STATE_LOGIN_CURL_UNSTUCK );
		RTNENUM( STATE_LOGIN_PROCESS_RESPONSE );
		RTNENUM( STATE_WORLD_INIT );
		RTNENUM( STATE_MULTIMEDIA_INIT );
		RTNENUM( STATE_FONT_INIT );
		RTNENUM( STATE_SEED_GRANTED_WAIT );
		RTNENUM( STATE_SEED_CAP_GRANTED );
		RTNENUM( STATE_WORLD_WAIT );
		RTNENUM( STATE_AGENT_SEND );
		RTNENUM( STATE_AGENT_WAIT );
		RTNENUM( STATE_INVENTORY_SEND );
		RTNENUM( STATE_MISC );
		RTNENUM( STATE_PRECACHE );
		RTNENUM( STATE_WEARABLES_WAIT );
		RTNENUM( STATE_CLEANUP );
		RTNENUM( STATE_STARTED );
		// <FS:Ansariel> Add FS-specific startup states
		RTNENUM( STATE_FETCH_GRID_INFO );
		RTNENUM( STATE_AUDIO_INIT);
		RTNENUM( STATE_AGENTS_WAIT );
		RTNENUM( STATE_LOGIN_CONFIRM_NOTIFICATON );
		// </FS:Ansariel>
	default:
		return llformat("(state #%d)", state);
	}
#undef RTNENUM
}

// static
void LLStartUp::setStartupState( EStartupState state )
{
	LL_INFOS("AppInit") << "Startup state changing from " <<  
		getStartupStateString() << " to " <<  
		startupStateToString(state) << LL_ENDL;

	getPhases().stopPhase(getStartupStateString());
	gStartupState = state;
	getPhases().startPhase(getStartupStateString());

	postStartupState();
}

void LLStartUp::postStartupState()
{
	LLSD stateInfo;
	stateInfo["str"] = getStartupStateString();
	stateInfo["enum"] = gStartupState;
	sStateWatcher->post(stateInfo);
	gDebugInfo["StartupState"] = getStartupStateString();
}


void reset_login()
{
	gAgentWearables.cleanup();
	gAgentCamera.cleanup();
	gAgent.cleanup();
	LLWorld::getInstance()->destroyClass();

	if ( gViewerWindow )
	{	// Hide menus and normal buttons
		gViewerWindow->setNormalControlsVisible( FALSE );
		gLoginMenuBarView->setVisible( TRUE );
		gLoginMenuBarView->setEnabled( TRUE );
	}

	// Hide any other stuff
	LLNotificationsUI::LLScreenChannelBase* chat_channel = LLNotificationsUI::LLChannelManager::getInstance()->findChannelByID(LLUUID(gSavedSettings.getString("NearByChatChannelUUID")));
	if(chat_channel)
	{
		chat_channel->removeToastsFromChannel();
	}
	LLFloaterReg::hideVisibleInstances();
    LLStartUp::setStartupState( STATE_BROWSER_INIT );
}

//---------------------------------------------------------------------------

// Initialize all plug-ins except the web browser (which was initialized
// early, before the login screen). JC
void LLStartUp::multimediaInit()
{
	LL_DEBUGS("AppInit") << "Initializing Multimedia...." << LL_ENDL;
	std::string msg = LLTrans::getString("LoginInitializingMultimedia");
	set_startup_status(0.42f, msg.c_str(), gAgent.mMOTD.c_str());
	display_startup();

	// Also initialise the stream titles.
	new StreamTitleDisplay();
}

void LLStartUp::fontInit()
{
	LL_DEBUGS("AppInit") << "Initializing fonts...." << LL_ENDL;
	std::string msg = LLTrans::getString("LoginInitializingFonts");
	set_startup_status(0.45f, msg.c_str(), gAgent.mMOTD.c_str());
	display_startup();

	LLFontGL::loadDefaultFonts();
}

void LLStartUp::initNameCache()
{
	// Can be called multiple times
	if ( gCacheName ) return;

	gCacheName = new LLCacheName(gMessageSystem);
	gCacheName->addObserver(&callback_cache_name);
	gCacheName->localizeCacheName("waiting", LLTrans::getString("AvatarNameWaiting"));
	gCacheName->localizeCacheName("nobody", LLTrans::getString("AvatarNameNobody"));
	gCacheName->localizeCacheName("none", LLTrans::getString("GroupNameNone"));
	// Load stored cache if possible
	LLAppViewer::instance()->loadNameCache();

	// Start cache in not-running state until we figure out if we have
	// capabilities for display name lookup
	LLAvatarNameCache* cache_inst = LLAvatarNameCache::getInstance();
	cache_inst->setUsePeopleAPI(gSavedSettings.getBOOL("UsePeopleAPI"));
	cache_inst->setUseDisplayNames(gSavedSettings.getBOOL("UseDisplayNames"));
	cache_inst->setUseUsernames(gSavedSettings.getBOOL("NameTagShowUsernames"));

	// <FS:CR> Legacy name/Username format
	LLAvatarName::setUseLegacyFormat(gSavedSettings.getBOOL("FSNameTagShowLegacyUsernames"));
	// <FS:CR> FIRE-6659: Legacy "Resident" name toggle
	LLAvatarName::setTrimResidentSurname(gSavedSettings.getBOOL("FSTrimLegacyNames"));
}


void LLStartUp::initExperiences()
{   
    // Should trigger loading the cache.
    LLExperienceCache::instance().setCapabilityQuery(
        boost::bind(&LLAgent::getRegionCapability, &gAgent, _1));

	LLExperienceLog::instance().initialize();
}

void LLStartUp::cleanupNameCache()
{
	delete gCacheName;
	gCacheName = NULL;
}

bool LLStartUp::dispatchURL()
{
	// ok, if we've gotten this far and have a startup URL
    if (!getStartSLURL().isValid())
	{
	  return false;
	}
    if(getStartSLURL().getType() != LLSLURL::APP)
	{
	    
		// If we started with a location, but we're already
		// at that location, don't pop dialogs open.
		LLVector3 pos = gAgent.getPositionAgent();
		LLVector3 slurlpos = getStartSLURL().getPosition();
		F32 dx = pos.mV[VX] - slurlpos.mV[VX];
		F32 dy = pos.mV[VY] - slurlpos.mV[VY];
		const F32 SLOP = 2.f;	// meters

		if( getStartSLURL().getRegion() != gAgent.getRegion()->getName()
			|| (dx*dx > SLOP*SLOP)
			|| (dy*dy > SLOP*SLOP) )
		{
			LLURLDispatcher::dispatch(getStartSLURL().getSLURLString(), "clicked",
						  NULL, false);
		}
		return true;
	}
	return false;
}

void LLStartUp::setStartSLURL(const LLSLURL& slurl) 
{
  sStartSLURL = slurl;
  switch(slurl.getType())
    {
    case LLSLURL::HOME_LOCATION:
      {
		  gSavedSettings.setString("LoginLocation", LLSLURL::SIM_LOCATION_HOME);
	break;
      }
    case LLSLURL::LAST_LOCATION:
      {
	gSavedSettings.setString("LoginLocation", LLSLURL::SIM_LOCATION_LAST);
	break;
      }
	// <FS:Ansariel> Support adding grids via SLURL
#if OPENSIM && !SINGLEGRID
	case LLSLURL::APP:
		if (slurl.getAppCmd() == "gridmanager")
		{
			LLURLDispatcher::dispatch(getStartSLURL().getSLURLString(), "clicked", NULL, false);
			break;
		}
#endif
	// </FS:Ansariel>
    default:
			LLGridManager::getInstance()->setGridChoice(slurl.getGrid());
			break;
    }

  // <FS:Ansariel> FIRE-29994: Start location doesn't get updated when selection a destination from the login splash screen
  FSPanelLogin::onUpdateStartSLURL(sStartSLURL);
}

// static
LLSLURL& LLStartUp::getStartSLURL()
{
	return sStartSLURL;
} 

/**
 * Read all proxy configuration settings and set up both the HTTP proxy and
 * SOCKS proxy as needed.
 *
 * Any errors that are encountered will result in showing the user a notification.
 * When an error is encountered,
 *
 * @return Returns true if setup was successful, false if an error was encountered.
 */
bool LLStartUp::startLLProxy()
{
	bool proxy_ok = true;
	std::string httpProxyType = gSavedSettings.getString("HttpProxyType");

	// Set up SOCKS proxy (if needed)
	if (gSavedSettings.getBOOL("Socks5ProxyEnabled"))
	{	
		// Determine and update LLProxy with the saved authentication system
		std::string auth_type = gSavedSettings.getString("Socks5AuthType");

		if (auth_type.compare("UserPass") == 0)
		{
			LLPointer<LLCredential> socks_cred = gSecAPIHandler->loadCredential("SOCKS5");
			std::string socks_user = socks_cred->getIdentifier()["username"].asString();
			std::string socks_password = socks_cred->getAuthenticator()["creds"].asString();

			bool ok = LLProxy::getInstance()->setAuthPassword(socks_user, socks_password);

			if (!ok)
			{
				LLNotificationsUtil::add("SOCKS_BAD_CREDS");
				proxy_ok = false;
			}
		}
		else if (auth_type.compare("None") == 0)
		{
			LLProxy::getInstance()->setAuthNone();
		}
		else
		{
			LL_WARNS("Proxy") << "Invalid SOCKS 5 authentication type."<< LL_ENDL;

			// Unknown or missing setting.
			gSavedSettings.setString("Socks5AuthType", "None");

			// Clear the SOCKS credentials.
			LLPointer<LLCredential> socks_cred = new LLCredential("SOCKS5");
			gSecAPIHandler->deleteCredential(socks_cred);

			LLProxy::getInstance()->setAuthNone();
		}

		if (proxy_ok)
		{
			// Start the proxy and check for errors
			// If status != SOCKS_OK, stopSOCKSProxy() will already have been called when startSOCKSProxy() returns.
			LLHost socks_host;
			socks_host.setHostByName(gSavedSettings.getString("Socks5ProxyHost"));
			socks_host.setPort(gSavedSettings.getU32("Socks5ProxyPort"));
			int status = LLProxy::getInstance()->startSOCKSProxy(socks_host);

			if (status != SOCKS_OK)
			{
				LLSD subs;
				subs["HOST"] = gSavedSettings.getString("Socks5ProxyHost");
				subs["PORT"] = (S32)gSavedSettings.getU32("Socks5ProxyPort");

				std::string error_string;

				switch(status)
				{
					case SOCKS_CONNECT_ERROR: // TCP Fail
						error_string = "SOCKS_CONNECT_ERROR";
						break;

					case SOCKS_NOT_PERMITTED: // SOCKS 5 server rule set refused connection
						error_string = "SOCKS_NOT_PERMITTED";
						break;

					case SOCKS_NOT_ACCEPTABLE: // Selected authentication is not acceptable to server
						error_string = "SOCKS_NOT_ACCEPTABLE";
						break;

					case SOCKS_AUTH_FAIL: // Authentication failed
						error_string = "SOCKS_AUTH_FAIL";
						break;

					case SOCKS_UDP_FWD_NOT_GRANTED: // UDP forward request failed
						error_string = "SOCKS_UDP_FWD_NOT_GRANTED";
						break;

					case SOCKS_HOST_CONNECT_FAILED: // Failed to open a TCP channel to the socks server
						error_string = "SOCKS_HOST_CONNECT_FAILED";
						break;

					case SOCKS_INVALID_HOST: // Improperly formatted host address or port.
						error_string = "SOCKS_INVALID_HOST";
						break;

					default:
						error_string = "SOCKS_UNKNOWN_STATUS"; // Something strange happened,
						LL_WARNS("Proxy") << "Unknown return from LLProxy::startProxy(): " << status << LL_ENDL;
						break;
				}

				LLNotificationsUtil::add(error_string, subs);
				proxy_ok = false;
			}
		}
	}
	else
	{
		LLProxy::getInstance()->stopSOCKSProxy(); // ensure no UDP proxy is running and it's all cleaned up
	}

	if (proxy_ok)
	{
		// Determine the HTTP proxy type (if any)
		if ((httpProxyType.compare("Web") == 0) && gSavedSettings.getBOOL("BrowserProxyEnabled"))
		{
			LLHost http_host;
			http_host.setHostByName(gSavedSettings.getString("BrowserProxyAddress"));
			http_host.setPort(gSavedSettings.getS32("BrowserProxyPort"));
			if (!LLProxy::getInstance()->enableHTTPProxy(http_host, LLPROXY_HTTP))
			{
				LLSD subs;
				subs["HOST"] = http_host.getIPString();
				subs["PORT"] = (S32)http_host.getPort();
				LLNotificationsUtil::add("PROXY_INVALID_HTTP_HOST", subs);
				proxy_ok = false;
			}
		}
		else if ((httpProxyType.compare("Socks") == 0) && gSavedSettings.getBOOL("Socks5ProxyEnabled"))
		{
			LLHost socks_host;
			socks_host.setHostByName(gSavedSettings.getString("Socks5ProxyHost"));
			socks_host.setPort(gSavedSettings.getU32("Socks5ProxyPort"));
			if (!LLProxy::getInstance()->enableHTTPProxy(socks_host, LLPROXY_SOCKS))
			{
				LLSD subs;
				subs["HOST"] = socks_host.getIPString();
				subs["PORT"] = (S32)socks_host.getPort();
				LLNotificationsUtil::add("PROXY_INVALID_SOCKS_HOST", subs);
				proxy_ok = false;
			}
		}
		else if (httpProxyType.compare("None") == 0)
		{
			LLProxy::getInstance()->disableHTTPProxy();
		}
		else
		{
			LL_WARNS("Proxy") << "Invalid other HTTP proxy configuration: " << httpProxyType << LL_ENDL;

			// Set the missing or wrong configuration back to something valid.
			gSavedSettings.setString("HttpProxyType", "None");
			LLProxy::getInstance()->disableHTTPProxy();

			// Leave proxy_ok alone, since this isn't necessarily fatal.
		}
	}

	return proxy_ok;
}

bool login_alert_done(const LLSD& notification, const LLSD& response)
{
	// <FS:Ansariel> [FS Login Panel]
	//LLPanelLogin::giveFocus();
	transition_back_to_login_panel(std::string());
	// </FS:Ansariel> [FS Login Panel]
	return false;
}

// parse the certificate information into args for the 
// certificate notifications
LLSD transform_cert_args(LLPointer<LLCertificate> cert)
{
	LLSD args = LLSD::emptyMap();
	std::string value;
	LLSD cert_info;
	cert->getLLSD(cert_info);
	// convert all of the elements in the cert into                                        
	// args for the xml dialog, so we have flexability to                                  
	// display various parts of the cert by only modifying                                 
	// the cert alert dialog xml.                                                          
	for(LLSD::map_iterator iter = cert_info.beginMap();
		iter != cert_info.endMap();
		iter++)
	{
		// key usage and extended key usage                                            
		// are actually arrays, and we want to format them as comma separated          
		// strings, so special case those.                                             
		LLSDSerialize::toXML(cert_info[iter->first], std::cout);
		if((iter->first== std::string(CERT_KEY_USAGE)) |
		   (iter->first == std::string(CERT_EXTENDED_KEY_USAGE)))
		{
			value = "";
			LLSD usage = cert_info[iter->first];
			for (LLSD::array_iterator usage_iter = usage.beginArray();
				 usage_iter != usage.endArray();
				 usage_iter++)
			{
				
				if(usage_iter != usage.beginArray())
				{
					value += ", ";
				}
				
				value += (*usage_iter).asString();
			}
			
		}
		else
		{
			value = iter->second.asString();
		}
		
		std::string name = iter->first;
		std::transform(name.begin(), name.end(), name.begin(),
					   (int(*)(int))toupper);
		args[name.c_str()] = value;
	}
	return args;
}


// when we handle a cert error, give focus back to the login panel
void general_cert_done(const LLSD& notification, const LLSD& response)
{
	// <FS:Ansariel> [FS Login Panel]
	//LLStartUp::setStartupState( STATE_LOGIN_SHOW );			
	//LLPanelLogin::giveFocus();
	transition_back_to_login_panel(std::string());
	// </FS:Ansariel> [FS Login Panel]
}

// check to see if the user wants to trust the cert.
// if they do, add it to the cert store and 
void trust_cert_done(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);	
	switch(option)
	{
		case OPT_TRUST_CERT:
		{
			LLPointer<LLCertificate> cert = gSecAPIHandler->getCertificate(notification["payload"]["certificate"]);
			LLPointer<LLCertificateStore> store = gSecAPIHandler->getCertificateStore(gSavedSettings.getString("CertStore"));			
			store->add(cert);
			store->save();
			LLStartUp::setStartupState( STATE_LOGIN_CLEANUP );	
			break;
		}
		case OPT_CANCEL_TRUST:
			// <FS:Ansariel> That's what transition_back_to_login_panel is for and does!
			//reset_login();
			//gSavedSettings.setBOOL("AutoLogin", FALSE);			
			//LLStartUp::setStartupState( STATE_LOGIN_SHOW );				
			transition_back_to_login_panel(std::string());
			// </FS:Ansariel>
		default:
			// <FS:Ansariel> [FS Login Panel]
			//LLPanelLogin::giveFocus();
			transition_back_to_login_panel(std::string());
			// </FS:Ansariel> [FS Login Panel]
			break;
	}

}

void apply_udp_blacklist(const std::string& csv)
{

	std::string::size_type start = 0;
	std::string::size_type comma = 0;
	do 
	{
		comma = csv.find(",", start);
		if (comma == std::string::npos)
		{
			comma = csv.length();
		}
		std::string item(csv, start, comma-start);

		LL_DEBUGS() << "udp_blacklist " << item << LL_ENDL;
		gMessageSystem->banUdpMessage(item);
		
		start = comma + 1;

	}
	while(comma < csv.length());
	
}

void on_benefits_failed_callback(const LLSD& notification, const LLSD& response)
{
	LL_WARNS("Benefits") << "Failed to load benefits information" << LL_ENDL; 
}

bool init_benefits(LLSD& response)
{
	bool succ = true;

	std::string package_name = response["account_type"].asString();
	const LLSD& benefits_sd = response["account_level_benefits"];
	if (!LLAgentBenefitsMgr::init(package_name, benefits_sd) ||
		!LLAgentBenefitsMgr::initCurrent(package_name, benefits_sd))
	{
		succ = false;
	}
	else
	{
		LL_DEBUGS("Benefits") << "Initialized current benefits, level " << package_name << " from " << benefits_sd << LL_ENDL;
	}
	const LLSD& packages_sd = response["premium_packages"];
	for(LLSD::map_const_iterator package_iter = packages_sd.beginMap();
		package_iter != packages_sd.endMap();
		++package_iter)
	{
		std::string package_name = package_iter->first;
		const LLSD& benefits_sd = package_iter->second["benefits"];
		if (LLAgentBenefitsMgr::init(package_name, benefits_sd))
		{
			LL_DEBUGS("Benefits") << "Initialized benefits for package " << package_name << " from " << benefits_sd << LL_ENDL;
		}
		else
		{
			LL_WARNS("Benefits") << "Failed init for package " << package_name << " from " << benefits_sd << LL_ENDL;
			succ = false;
		}
	}

	if (!LLAgentBenefitsMgr::has("Base"))
	{
		LL_WARNS("Benefits") << "Benefits info did not include required package Base" << LL_ENDL;
		succ = false;
	}
	if (!LLAgentBenefitsMgr::has("Premium"))
	{
		LL_WARNS("Benefits") << "Benefits info did not include required package Premium" << LL_ENDL;
		succ = false;
	}

	// FIXME PREMIUM - for testing if login does not yet provide Premium Plus. Should be removed thereafter.
	//if (succ && !LLAgentBenefitsMgr::has("Premium Plus"))
	//{
	//	LLAgentBenefitsMgr::init("Premium Plus", packages_sd["Premium"]["benefits"]);
	//	llassert(LLAgentBenefitsMgr::has("Premium Plus"));
	//}
	return succ;
}

// <FS:CR> Aurora Sim
//bool process_login_success_response()
bool process_login_success_response(U32 &first_sim_size_x, U32 &first_sim_size_y)
// </FS:CR> Aurora Sim
{
	LLSD response = LLLoginInstance::getInstance()->getResponse();

	// <FS:Ansariel> OpenSim legacy economy support
	//mBenefitsSuccessfullyInit = init_benefits(response);
	if (LLGridManager::instance().isInSecondLife())
	{
		mBenefitsSuccessfullyInit = init_benefits(response);
	}
	else
	{
		mBenefitsSuccessfullyInit = true;
	}
	// </FS:Ansariel>

	std::string text(response["udp_blacklist"]);
	if(!text.empty())
	{
		apply_udp_blacklist(text);
	}

	// unpack login data needed by the application
	text = response["agent_id"].asString();
	if(!text.empty()) gAgentID.set(text);
//	gDebugInfo["AgentID"] = text;
// [SL:KB] - Patch: Viewer-CrashReporting | Checked: 2010-11-16 (Catznip-2.6.0a) | Added: Catznip-2.4.0b
	if (gCrashSettings.getBOOL("CrashSubmitName"))
	{
		// Only include the agent UUID if the user consented
		gDebugInfo["AgentID"] = text;
	}
// [/SL:KB]
	
	// Agent id needed for parcel info request in LLUrlEntryParcel
	// to resolve parcel name.
	LLUrlEntryParcel::setAgentID(gAgentID);

	text = response["session_id"].asString();
	if(!text.empty()) gAgentSessionID.set(text);
//	gDebugInfo["SessionID"] = text;

	// Session id needed for parcel info request in LLUrlEntryParcel
	// to resolve parcel name.
	LLUrlEntryParcel::setSessionID(gAgentSessionID);
	
	text = response["secure_session_id"].asString();
	if(!text.empty()) gAgent.mSecureSessionID.set(text);

	// if the response contains a display name, use that,
	// otherwise if the response contains a first and/or last name,
	// use those.  Otherwise use the credential identifier

	gDisplayName = "";
	if (response.has("display_name"))
	{
		gDisplayName.assign(response["display_name"].asString());
		if(!gDisplayName.empty())
		{
			// Remove quotes from string.  Login.cgi sends these to force
			// names that look like numbers into strings.
			LLStringUtil::replaceChar(gDisplayName, '"', ' ');
			LLStringUtil::trim(gDisplayName);
		}
	}
	std::string first_name;
	if(response.has("first_name"))
	{
		first_name = response["first_name"].asString();
		LLStringUtil::replaceChar(first_name, '"', ' ');
		LLStringUtil::trim(first_name);
		gAgentUsername = first_name;
	}

	if(response.has("last_name") && !gAgentUsername.empty())
	{
		std::string last_name = response["last_name"].asString();
		if (last_name != "Resident")
		{
		    LLStringUtil::replaceChar(last_name, '"', ' ');
		    LLStringUtil::trim(last_name);
		    gAgentUsername = gAgentUsername + " " + last_name;
		}
	}

	if(gDisplayName.empty())
	{
		if(response.has("first_name"))
		{
			gDisplayName.assign(response["first_name"].asString());
			LLStringUtil::replaceChar(gDisplayName, '"', ' ');
			LLStringUtil::trim(gDisplayName);
		}
		if(response.has("last_name"))
		{
			text.assign(response["last_name"].asString());
			LLStringUtil::replaceChar(text, '"', ' ');
			LLStringUtil::trim(text);
			if(!gDisplayName.empty())
			{
				gDisplayName += " ";
			}
			gDisplayName += text;
		}
	}

	if(gDisplayName.empty())
	{
		gDisplayName.assign(gUserCredential->asString());
	}

	// this is their actual ability to access content
	text = response["agent_access_max"].asString();
	if (!text.empty())
	{
		// agent_access can be 'A', 'M', and 'PG'.
		gAgent.setMaturity(text[0]);
		//<FS:TS> FIRE-8854: Set the preferred maturity here to the maximum
		//        in case the sim doesn't send it at login, like OpenSim 
		//        doesn't. If it does, it'll get overridden below.
		U32 preferredMaturity = (U32)LLAgent::convertTextToMaturity(text[0]);
		gSavedSettings.setU32("PreferredMaturity", preferredMaturity);
	}
	
	// this is the value of their preference setting for that content
	// which will always be <= agent_access_max
	text = response["agent_region_access"].asString();
	if (!text.empty())
	{
		U32 preferredMaturity = (U32)LLAgent::convertTextToMaturity(text[0]);
		gSavedSettings.setU32("PreferredMaturity", preferredMaturity);
	}

	text = response["start_location"].asString();
	if(!text.empty()) 
	{
		gAgentStartLocation.assign(text);
	}

	text = response["circuit_code"].asString();
	if(!text.empty())
	{
		gMessageSystem->mOurCircuitCode = strtoul(text.c_str(), NULL, 10);
	}
	std::string sim_ip_str = response["sim_ip"];
	std::string sim_port_str = response["sim_port"];
	if(!sim_ip_str.empty() && !sim_port_str.empty())
	{
		U32 sim_port = strtoul(sim_port_str.c_str(), NULL, 10);
		gFirstSim.set(sim_ip_str, sim_port);
		if (gFirstSim.isOk())
		{
			gMessageSystem->enableCircuit(gFirstSim, TRUE);
		}
	}
	std::string region_x_str = response["region_x"];
	std::string region_y_str = response["region_y"];
	if(!region_x_str.empty() && !region_y_str.empty())
	{
		U32 region_x = strtoul(region_x_str.c_str(), NULL, 10);
		U32 region_y = strtoul(region_y_str.c_str(), NULL, 10);
		gFirstSimHandle = to_region_handle(region_x, region_y);
	}
	
// <FS:CR> Aurora Sim
	text = response["region_size_x"].asString();
	if(!text.empty()) {
		first_sim_size_x = strtoul(text.c_str(), NULL, 10);
		LLViewerParcelMgr::getInstance()->init(first_sim_size_x);
	}

	//region Y size is currently unused, major refactoring required. - Patrick Sapinski (2/10/2011)
	text = response["region_size_y"].asString();
	if(!text.empty()) first_sim_size_y = strtoul(text.c_str(), NULL, 10);
// </FS:CR> Aurora Sim	
	const std::string look_at_str = response["look_at"];
	if (!look_at_str.empty())
	{
		size_t len = look_at_str.size();
		LLMemoryStream mstr((U8*)look_at_str.c_str(), len);
		LLSD sd = LLSDSerialize::fromNotation(mstr, len);
		gAgentStartLookAt = ll_vector3_from_sd(sd);
	}

	text = response["seed_capability"].asString();
	if (!text.empty()) gFirstSimSeedCap = text;
				
	text = response["seconds_since_epoch"].asString();
	if(!text.empty())
	{
		U32 server_utc_time = strtoul(text.c_str(), NULL, 10);
		if(server_utc_time)
		{
			time_t now = time(NULL);
			gUTCOffset = (server_utc_time - now);

			// Print server timestamp
			LLSD substitution;
			substitution["datetime"] = (S32)server_utc_time;
			std::string timeStr = "[month, datetime, slt] [day, datetime, slt] [year, datetime, slt] [hour, datetime, slt]:[min, datetime, slt]:[second, datetime, slt]";
			LLStringUtil::format(timeStr, substitution);
			LL_INFOS("AppInit") << "Server SLT timestamp: " << timeStr << ". Server-viewer time offset before correction: " << gUTCOffset << "s" << LL_ENDL;
		}
	}

	// this is the base used to construct help URLs
	text = response["help_url_format"].asString();
	if (!text.empty())
	{
		// replace the default help URL format
		gSavedSettings.setString("HelpURLFormat",text);
	}

	std::string home_location = response["home"];
	if(!home_location.empty())
	{
		size_t len = home_location.size();
		LLMemoryStream mstr((U8*)home_location.c_str(), len);
		LLSD sd = LLSDSerialize::fromNotation(mstr, len);
		S32 region_x = sd["region_handle"][0].asInteger();
		S32 region_y = sd["region_handle"][1].asInteger();
		U64 region_handle = to_region_handle(region_x, region_y);
		LLVector3 position = ll_vector3_from_sd(sd["position"]);
		gAgent.setHomePosRegion(region_handle, position);
	}

	// If MOTD has not been set by fsdata, fallback to LL MOTD
// <FS:CR> FIRE-8571, FIRE-9274
	if (gAgent.mMOTD.empty() || !LLGridManager::getInstance()->isInSLMain())
// </FS:CR>
	{
		gAgent.mMOTD.assign(response["message"]);
	}
	
	// <FS:Techwolf Lupindo> fsdata opensim MOTD support
#ifdef OPENSIM
	if (LLGridManager::getInstance()->isInOpenSim() && !FSData::instance().getOpenSimMOTD().empty())
	{
		gAgent.mMOTD.assign(FSData::instance().getOpenSimMOTD());
	}
#endif
	// </FS:Techwolf Lupindo>

	// Options...
	// Each 'option' is an array of submaps. 
	// It appears that we only ever use the first element of the array.
	LLUUID inv_root_folder_id = response["inventory-root"][0]["folder_id"];
	if(inv_root_folder_id.notNull())
	{
		gInventory.setRootFolderID(inv_root_folder_id);
		//gInventory.mock(gAgent.getInventoryRootID());
	}

	LLSD login_flags = response["login-flags"][0];
	if(login_flags.size())
	{
		std::string flag = login_flags["ever_logged_in"];
		if(!flag.empty())
		{
			gAgent.setFirstLogin((flag == "N") ? TRUE : FALSE);
		}

		/*  Flag is currently ignored by the viewer.
		flag = login_flags["stipend_since_login"];
		if(flag == "Y") 
		{
			stipend_since_login = true;
		}
		*/

		flag = login_flags["gendered"].asString();
		if(flag == "Y")
		{
			// We don't care about this flag anymore; now base whether
			// outfit is chosen on COF contents, initial outfit
			// requested and available, etc.

			//gAgent.setGenderChosen(TRUE);
		}
		
		bool pacific_daylight_time = false;
		flag = login_flags["daylight_savings"].asString();
		if(flag == "Y")
		{
			pacific_daylight_time = (flag == "Y");
		}

		//setup map of datetime strings to codes and slt & local time offset from utc
		LLStringOps::setupDatetimeInfo(pacific_daylight_time);
	}

	// set up the voice configuration.  Ultimately, we should pass this up as part of each voice
	// channel if we need to move to multiple voice servers per grid.
	LLSD voice_config_info = response["voice-config"];
	if(voice_config_info.has("VoiceServerType"))
	{
		gSavedSettings.setString("VoiceServerType", voice_config_info["VoiceServerType"].asString()); 
	}

	// Request the map server url
	std::string map_server_url = response["map-server-url"];
	if(!map_server_url.empty())
	{
		// We got an answer from the grid -> use that for map for the current session
		gSavedSettings.setString("CurrentMapServerURL", map_server_url); 
		LL_INFOS("LLStartup") << "map-server-url : we got an answer from the grid : " << map_server_url << LL_ENDL;
	}
	else
	{
		// No answer from the grid -> use the default setting for current session 
		map_server_url = gSavedSettings.getString("MapServerURL"); 
		gSavedSettings.setString("CurrentMapServerURL", map_server_url); 
		LL_INFOS("LLStartup") << "map-server-url : no map-server-url answer, we use the default setting for the map : " << map_server_url << LL_ENDL;
	}
	
// <FS:CR> FIRE-8063: Read Aurora web profile url from login data
#ifdef OPENSIM
	std::string web_profile_url = response["web_profile_url"];
	if (!web_profile_url.empty())
	{
		// We got an answer from the grid -> use that for map for the current session
		LLGridManager::instance().setWebProfileUrl(web_profile_url);
		LL_INFOS("LLStartup") << "web-profile-url : we got an answer from the grid : " << web_profile_url << LL_ENDL;
	}
// <FS:CR> FIRE-10567 - Set classified fee, if it's available.
	if (response.has("classified_fee"))
	{
		S32 classified_fee = response["classified_fee"];
		LLGridManager::getInstance()->setClassifiedFee(classified_fee);
	}
	else
	{
		LLGridManager::getInstance()->setClassifiedFee(0);	// Free is a sensible default
	}
// <FS:CR> Set a parcel listing fee, if it's available
	if (response.has("directory_fee"))
	{
		S32 directory_fee = response["directory_fee"];
		LLGridManager::getInstance()->setDirectoryFee(directory_fee);
	}
	else
	{
		LLGridManager::getInstance()->setDirectoryFee(0);
	}
#endif // OPENSIM
// </FS:CR>
	// Default male and female avatars allowing the user to choose their avatar on first login.
	// These may be passed up by SLE to allow choice of enterprise avatars instead of the standard
	// "new ruth."  Not to be confused with 'initial-outfit' below 
	LLSD newuser_config = response["newuser-config"][0];
	if(newuser_config.has("DefaultFemaleAvatar"))
	{
		gSavedSettings.setString("DefaultFemaleAvatar", newuser_config["DefaultFemaleAvatar"].asString()); 		
	}
	if(newuser_config.has("DefaultMaleAvatar"))
	{
		gSavedSettings.setString("DefaultMaleAvatar", newuser_config["DefaultMaleAvatar"].asString()); 		
	}
	
	// Initial outfit for the user.
	LLSD initial_outfit = response["initial-outfit"][0];
	if(initial_outfit.size())
	{
		std::string flag = initial_outfit["folder_name"];
		if(!flag.empty())
		{
			// Initial outfit is a folder in your inventory,
			// must be an exact folder-name match.
			sInitialOutfit = flag;
		}

		flag = initial_outfit["gender"].asString();
		if(!flag.empty())
		{
			sInitialOutfitGender = flag;
		}
	}

	// set the location of the Agent Appearance service, from which we can request
	// avatar baked textures if they are supported by the current region
	std::string agent_appearance_url = response["agent_appearance_service"];
	if (!agent_appearance_url.empty())
	{
		LLAppearanceMgr::instance().setAppearanceServiceURL(agent_appearance_url);
	}

	// Set the location of the snapshot sharing config endpoint
	// <FS:Ansariel> Debug setting doesn't exist anymore as of 14-09-2014
	//std::string snapshot_config_url = response["snapshot_config_url"];
	//if(!snapshot_config_url.empty())
	//{
	//	gSavedSettings.setString("SnapshotConfigURL", snapshot_config_url);
	//}
	// </FS:Ansariel>

	// Start the process of fetching the OpenID session cookie for this user login
	std::string openid_url = response["openid_url"];
	if(!openid_url.empty())
	{
		std::string openid_token = response["openid_token"];
		LLViewerMedia::getInstance()->openIDSetup(openid_url, openid_token);
	}

	// <FS:Ansariel> OpenSim legacy economy support
#ifdef OPENSIM
	if (!LLGridManager::instance().isInSecondLife())
	{
		if (response.has("max-agent-groups") || response.has("max_groups"))
		{
			std::string max_agent_groups;
			response.has("max_groups") ?
				max_agent_groups = response["max_groups"].asString()
				: max_agent_groups = response["max-agent-groups"].asString();

			gMaxAgentGroups = atoi(max_agent_groups.c_str());
			LL_INFOS("LLStartup") << "gMaxAgentGroups read from login.cgi: "
				<< gMaxAgentGroups << LL_ENDL;
		}
		else
		{
			gMaxAgentGroups = 0;
			LL_INFOS("LLStartup") << "did not receive max-agent-groups. unlimited groups activated" << LL_ENDL;
		}
	}
#endif
	// </FS:Ansariel>

	// <COLOSI opensim multi-currency support>
	std::string prev_currency_symbol = Tea::getCurrency();
	// </COLOSI opensim multi-currency support>

// <FS:AW opensim currency support>
	std::string currency = "L$";
#ifdef OPENSIM // <FS:AW optional opensim support>
	if(response.has("currency"))
	{
		currency = response["currency"].asString();
		LL_DEBUGS("OS_SETTINGS") << "currency " << currency << LL_ENDL;
	}
	else if (LLGridManager::getInstance()->isInOpenSim())
	{
		currency = "OS$";
		LL_DEBUGS("OS_SETTINGS") << "no currency in login response" << LL_ENDL;
	}
	Tea::setCurrency(currency);
// </FS:AW opensim currency support>

	// <COLOSI  opensim multi-currency support>
	// Blank out the region currency which is set in in lfsimfeatureshandler
	Tea::setRegionCurrency(LLStringUtil::null);
	std::string new_currency_symbol = Tea::getCurrency();
	// If currency symbol has changed, update currency symbols where manually necessary.
	if (new_currency_symbol != prev_currency_symbol)
	{
		LFSimFeatureHandler::updateCurrencySymbols();
	}
	// </COLOSI opensim multi-currency support>

// <FS:AW  opensim destinations and avatar picker>
	if(response.has("avatar_picker_url"))
	{
		LL_DEBUGS("OS_SETTINGS") << "avatar_picker_url " << response["avatar_picker_url"] << LL_ENDL;
	}
	else if (LLGridManager::getInstance()->isInOpenSim())
	{
		LL_DEBUGS("OS_SETTINGS") << "no avatar_picker_url in login response" << LL_ENDL;
	}

	if(response.has("destination_guide_url"))
	{
		LL_DEBUGS("OS_SETTINGS") << "destination_guide_url " << response["destination_guide_url"] << LL_ENDL;
	}
	else if (LLGridManager::getInstance()->isInOpenSim())
	{
		LL_DEBUGS("OS_SETTINGS") << "no destination_guide_url in login response" << LL_ENDL;
	}
// </FS:AW  opensim destinations and avatar picker>
	
// <FS:CR> Legacy search killswitch!
	if (LLGridManager::getInstance()->isInOpenSim())
	{
		LLFloaterReg::add("search", "floater_fs_search.xml", (LLFloaterBuildFunc)&LLFloaterReg::build<FSFloaterSearch>);
	}
	else
#endif // OPENSIM
	{
		if (FSData::instance().enableLegacySearch())
		{
			LLFloaterReg::add("search", "floater_fs_search.xml", (LLFloaterBuildFunc)&LLFloaterReg::build<FSFloaterSearch>);
		}
		else
		{
			LLFloaterReg::add("search", "floater_search.xml", (LLFloaterBuildFunc)&LLFloaterReg::build<LLFloaterSearch>);
		}
	}
// </FS:CR>

	// <FS:Techwolf Lupindo> fsdata support
	if (FSData::instance().isAgentFlag(gAgentID, FSData::NO_USE))
	{
		gAgentID.setNull();
	}
	// </FS:Techwolf Lupindo>

	bool success = false;
	// JC: gesture loading done below, when we have an asset system
	// in place.  Don't delete/clear gUserCredentials until then.
	if(gAgentID.notNull()
	   && gAgentSessionID.notNull()
	   && gMessageSystem->mOurCircuitCode
	   && gFirstSim.isOk()
	   && gInventory.getRootFolderID().notNull())
	{
		success = true;
	}
    LLAppViewer* pApp = LLAppViewer::instance();
	pApp->writeDebugInfo();     //Write our static data now that we have username, session_id, etc.
	return success;
}

void transition_back_to_login_panel(const std::string& emsg)
{
	// Bounce back to the login screen.
	reset_login(); // calls LLStartUp::setStartupState( STATE_LOGIN_SHOW );
	gSavedSettings.setBOOL("AutoLogin", FALSE);
}

// <FS:KC> FIRE-18250: Option to disable default eye movement
//static
void update_static_eyes()
{
	if (gSavedPerAccountSettings.getBOOL("FSStaticEyes"))
	{
		LLUUID anim_id(gSavedSettings.getString("FSStaticEyesUUID"));
		gAgent.sendAnimationRequest(anim_id, ANIM_REQUEST_START);
	}
}
// </FS:KC>
