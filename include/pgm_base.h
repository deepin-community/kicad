/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004-2015 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008-2015 Wayne Stambaugh <stambaughw@gmail.com>
 * Copyright (C) 1992-2021 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/**
 * @file pgm_base.h
 * @brief see class PGM_BASE
 */

#ifndef  PGM_BASE_H_
#define  PGM_BASE_H_

#include <exception>
#include <map>
#include <vector>
#include <memory>
#include <search_stack.h>
#include <settings/environment.h>
#include <wx/filename.h>
#include <wx/snglinst.h>

class wxApp;
class wxMenu;
class wxWindow;
class wxSplashScreen;

class BACKGROUND_JOBS_MONITOR;
class NOTIFICATIONS_MANAGER;
class COMMON_SETTINGS;
class SETTINGS_MANAGER;
class SCRIPTING;

/**
 * A small class to handle the list of existing translations.
 *
 * The locale translation is automatic.  The selection of languages is mainly for
 * maintainer's convenience.  To add a support to a new translation add a new item
 * to #LanguagesList[].
 */
struct LANGUAGE_DESCR
{
    /// wxWidgets locale identifier (See wxWidgets doc)
    int         m_WX_Lang_Identifier;

    /// KiCad identifier used in menu selection (See id.h)
    int         m_KI_Lang_Identifier;

    /// Labels used in menus
    wxString    m_Lang_Label;

    /// Set to true if the m_Lang_Label must not be translated
    bool        m_DoNotTranslate;
};


/**
 * An array containing all the languages that KiCad supports.
 */
extern LANGUAGE_DESCR LanguagesList[];

/**
 * Container for data for KiCad programs.
 *
 * The functions are virtual so we can do cross module calls without linking to them.  This
 * used to be a wxApp derivative, but that is difficult under wxPython which shapes the wxApp.
 * So now this is a "side-car" (like a motorcycle side-car) object with a back pointer into
 * the wxApp which initializes it.
 *
 * - OnPgmStart() is virtual, may be overridden, and parallels wxApp::OnInit(), from where it
 *   should called.
 * - OnPgmEnd() is virtual, may be overridden, and parallels wxApp::OnExit(), from where it
 *   should be called.
 */
class PGM_BASE
{
public:
    PGM_BASE();
    virtual ~PGM_BASE();

#if 0
    /*

    Derived classes must implement these two functions: OnPgmInit() and
    OnPgmExit(), and since they are only called from same source file as their
    implementation, these need not be virtual here. In fact, in the case of
    python project manager's class PGM_PYTHON, these functions are actually
    written in python. In total there are three implementations, corresponding
    to the three defines given by kiface.h's KFCTL_* #defines.

    */

    /**
     * This is the first executed function (like main() ).
     *
     * @return true if the application can be started.
     */
    virtual bool OnPgmInit() = 0;           // call this from wxApp::OnInit()

    virtual void OnPgmExit() = 0;           // call this from wxApp::OnExit()
#endif

    /**
     * Builds the UTF8 based argv variable
     */
    void BuildArgvUtf8();

    /**
     * Specific to MacOSX (not used under Linux or Windows).
     *
     * MacOSX requires it for file association.
     * @see http://wiki.wxwidgets.org/WxMac-specific_topics
     */
    virtual void MacOpenFile( const wxString& aFileName ) = 0;

    virtual SETTINGS_MANAGER& GetSettingsManager() const { return *m_settings_manager; }

    virtual COMMON_SETTINGS*  GetCommonSettings() const;

    virtual BACKGROUND_JOBS_MONITOR& GetBackgroundJobMonitor() const { return *m_background_jobs_monitor; }

    virtual NOTIFICATIONS_MANAGER& GetNotificationsManager() const { return *m_notifications_manager; }

    virtual void SetTextEditor( const wxString& aFileName );

    /**
     * Return the path to the preferred text editor application.
     *
     * @param   aCanShowFileChooser If no editor is currently set and this argument is
     *          'true' then this method will show a file chooser dialog asking for the
     *          editor's executable.
     * @return  Returns the full path of the editor, or an empty string if no editor has
     *          been set.
     */
    virtual const wxString& GetTextEditor( bool aCanShowFileChooser = true );

    /**
     * Shows a dialog that instructs the user to select a new preferred editor.
     * @param   aDefaultEditor Default full path for the default editor this dialog should
     *          show by default.
     * @return  Returns the full path of the editor, or an empty string if no editor was
     *          chosen.
     */
    virtual const wxString AskUserForPreferredEditor(
            const wxString& aDefaultEditor = wxEmptyString );

    virtual bool IsKicadEnvVariableDefined() const               { return !m_kicad_env.IsEmpty(); }

    virtual const wxString& GetKicadEnvVariable() const          { return m_kicad_env; }

    virtual const wxString& GetExecutablePath() const;

    virtual wxLocale* GetLocale()                                { return m_locale; }

    virtual const wxString& GetPdfBrowserName() const            { return m_pdf_browser; }

    virtual void SetPdfBrowserName( const wxString& aFileName )  { m_pdf_browser = aFileName; }

    /**
     * @return true if the PDF browser is the default (system) PDF browser and false if the
     *         PDF browser is the preferred (selected) browser, else returns false if there
     *         is no selected browser.
     */
    virtual bool UseSystemPdfBrowser() const
    {
        return m_use_system_pdf_browser || m_pdf_browser.IsEmpty();
    }

    /**
     * Force the use of system PDF browser, even if a preferred PDF browser is set.
     */
    virtual void ForceSystemPdfBrowser( bool aFlg ) { m_use_system_pdf_browser = aFlg; }

    /**
     * Set the dictionary file name for internationalization.
     *
     * The files are in kicad/internat/xx or kicad/internat/xx_XX and are named kicad.mo
     *
     * @param aErrMsg is the string to return the error message it.
     * @param first_time must be set to true the first time this function is called,
     *                   false otherwise.
     * @return false if there was an error setting the language.
     */
    virtual bool SetLanguage( wxString& aErrMsg, bool first_time = false );

    /**
     * Set the default language without reference to any preferences.  Can be used to set
     * the language for dialogs that show before preferences are loaded
     * @param aErrMsg String to return the error message(s) in
     * @return false if the language could not be set
     */
    bool SetDefaultLanguage( wxString& aErrMsg );

    /**
     * Set in .m_language_id member the wxWidgets language identifier ID from the KiCad
     * menu id (internal menu identifier).
     *
     * @param menu_id The KiCad menuitem id (returned by Menu Event, when clicking on a
     *                 menu item)
     */
    virtual void SetLanguageIdentifier( int menu_id );

    /**
     * @return the wxWidgets language identifier Id of the language currently selected.
     */
    virtual int GetSelectedLanguageIdentifier() const { return m_language_id; }

    /**
     * @return the current selected language in rfc3066 format
     */
    virtual wxString GetLanguageTag();

    virtual void SetLanguagePath();

    /**
     * Read the PDF browser choice from the common configuration.
     */
    virtual void ReadPdfBrowserInfos();

    /**
     * Save the PDF browser choice to the common configuration.
     */
    virtual void WritePdfBrowserInfos();

    /**
     * Sets the environment variable \a aName to \a aValue.
     *
     * This function first checks to see if the environment variable \a aName is already
     * defined.  If it is not defined, then the environment variable \a aName is set to
     * a value.  Otherwise, the environment variable is left unchanged.  This allows the user
     * to override environment variables for testing purposes.
     *
     * @param aName is a wxString containing the environment variable name.
     * @param aValue is a wxString containing the environment variable value.
     * @return true if the environment variable \a Name was set to \a aValue.
     */
    virtual bool SetLocalEnvVariable( const wxString& aName, const wxString& aValue );

    /**
     * Updates the local environment with the contents of the current ENV_VAR_MAP stored in the
     * COMMON_SETTINGS
     * @see GetLocalEnvVariables()
     */
    virtual void SetLocalEnvVariables();

    virtual ENV_VAR_MAP& GetLocalEnvVariables() const;

    /**
     * Returns a bare naked wxApp which may come from wxPython, SINGLE_TOP, or kicad.exe.
     *
     * This should return what wxGetApp() returns.
     */
    virtual wxApp&   App();

    static const wxChar workingDirKey[];

    /**
     * Initialize this program.
     *
     * Initialize the process in a KiCad standard way using some generalized techniques:
     *  - Default paths (help, libs, bin) and configuration file names
     *  - Language and locale
     *  - fonts
     *
     * @note Do not initialize anything relating to DSOs or projects.
     *
     * @param aHeadless If true, run in headless mode (e.g. for unit tests)
     * @param aSkipPyInit If true, do not init python stuff.
     * Useful in application that do not use python, to disable python dependency at run time
     * @return true if success, false if failure and program is to terminate.
     */
    bool InitPgm( bool aHeadless = false, bool aSkipPyInit = false, bool aIsUnitTest = false );

    // The PGM_* classes can have difficulties at termination if they
    // are not destroyed soon enough.  Relying on a static destructor can be
    // too late for contained objects like wxSingleInstanceChecker.
    void Destroy();

    /**
     * Save the program (process) settings subset which are stored .kicad_common.
     */
    void SaveCommonSettings();

#ifdef KICAD_USE_SENTRY
    /**
     * @return True if the user agreed to sentry data collection
     */
    bool IsSentryOptedIn();

    /**
     * Sets the Sentry opt in state, this will also terminate sentry
     * immediately if needed, however it will not init sentry if opted in
     *
     * @param aOptIn True/false to agreeing to the use of sentry
     */
    void SetSentryOptIn( bool aOptIn );

    /**
     * Generates and stores a new sentry id at random using the boost uuid generator
     */
    void ResetSentryId();

    /**
     * Gets the current id string being used as "user id" in sentry reports
     */
    const wxString& GetSentryId();
#endif

    /**
     * A exception handler to be used at the top level if exceptions bubble up that for
     *
     * The purpose is to have a central place to log a wxWidgets error message and/or sentry report
     *
     * @param aPtr Pass the std::current_exception() from within the catch block
     */
    void HandleException( std::exception_ptr aPtr );

    /**
     * A common assert handler to be used between single_top and kicad
     *
     * This lets us have a common set of assert handling, including triggering sentry reports
     *
     * @param aFile the file path of the assert
     * @param aLine the line number of the assert
     * @param aFunc the function name the assert is within
     * @param aCond the condition of the assert
     * @param aMsg the attached assert message (can be empty)
     */
    void HandleAssert( const wxString& aFile, int aLine, const wxString& aFunc,
                       const wxString& aCond, const wxString& aMsg );

    /**
     * Determine if the application is running with a GUI
     *
     * @return true if there is a GUI and false otherwise
     */
    bool IsGUI();


    void ShowSplash();
    void HideSplash();

    /**
     * Allows access to the wxSingleInstanceChecker to test for other running KiCads
    */
    std::unique_ptr<wxSingleInstanceChecker>& SingleInstance()
    {
        return m_pgm_checker;
    }

    /**
     * wxWidgets on MSW tends to crash if you spool up more than one print job at a time.
     */
    bool m_Printing;

    std::vector<void*> m_ModalDialogs;

    bool m_Quitting;

    bool m_PropertyGridInitialized;

protected:
    /// Loads internal settings from COMMON_SETTINGS
    void loadCommonSettings();

    /// Trap all changes in here, simplifies debugging
    void setLanguageId( int aId )       { m_language_id = aId; }

#ifdef KICAD_USE_SENTRY
    void     sentryInit();
    void     sentryPrompt();
    wxString sentryCreateUid();
#endif

protected:
    std::unique_ptr<SETTINGS_MANAGER> m_settings_manager;
    std::unique_ptr<BACKGROUND_JOBS_MONITOR> m_background_jobs_monitor;
    std::unique_ptr<NOTIFICATIONS_MANAGER> m_notifications_manager;

    std::unique_ptr<SCRIPTING> m_python_scripting;

    /// Checks if there is another copy of Kicad running at the same time
    std::unique_ptr<wxSingleInstanceChecker> m_pgm_checker;


    wxString        m_kicad_env;              /// The KICAD system environment variable.

    wxLocale*       m_locale;
    int             m_language_id;

    bool            m_use_system_pdf_browser;
    wxString        m_pdf_browser;            /// Filename of the app selected for browsing PDFs

    wxString        m_text_editor;

#ifdef KICAD_USE_SENTRY
    wxFileName      m_sentry_optin_fn;
    wxFileName      m_sentry_uid_fn;
    wxString        m_sentryUid;
#endif

    char** m_argvUtf8;                      /// argv parameters converted to utf8 form, because wxwidgets has opinions
                                            /// and will return argv as either force converted to ascii in char* or wchar_t only

    int m_argcUtf8;

    wxSplashScreen* m_splash;
};


/// The global Program "get" accessor.
/// Implemented in: 1) common/single_top.cpp,  2) kicad/kicad.cpp, and 3) scripting/kiway.i
extern PGM_BASE& Pgm();

/// similar to PGM_BASE& Pgm(), but return a reference that can be nullptr
/// when running a shared lib from a script, not from a kicad appl
extern PGM_BASE* PgmOrNull();


#endif  // PGM_BASE_H_
