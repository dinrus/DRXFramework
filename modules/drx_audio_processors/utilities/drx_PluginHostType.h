/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace drx
{

//==============================================================================
/**
     A useful utility class to determine the host or DAW in which your plugin is
     loaded.

     Declare a PluginHostType object in your class to use it.

    @tags{Audio}
*/
class PluginHostType
{
public:
    //==============================================================================
    PluginHostType()  : type (getHostType()) {}
    PluginHostType (const PluginHostType& other) = default;
    PluginHostType& operator= (const PluginHostType& other) = default;

    //==============================================================================
    /** Represents the host type and also its version for some hosts. */
    enum HostType
    {
        UnknownHost,                /**< Represents an unknown host. */
        AbletonLive6,               /**< Represents Ableton Live 6. */
        AbletonLive7,               /**< Represents Ableton Live 7. */
        AbletonLive8,               /**< Represents Ableton Live 8. */
        AbletonLive9,               /**< Represents Ableton Live 9. */
        AbletonLive10,              /**< Represents Ableton Live 10. */
        AbletonLive11,              /**< Represents Ableton Live 11. */
        AbletonLiveGeneric,         /**< Represents Ableton Live. */
        AdobeAudition,              /**< Represents Adobe Audition. */
        AdobePremierePro,           /**< Represents Adobe Premiere Pro. */
        AppleGarageBand,            /**< Represents Apple GarageBand. */
        AppleInfoHelper,            /**< Represents Apple com.apple.audio.InfoHelper. */
        AppleLogic,                 /**< Represents Apple Logic Pro. */
        AppleMainStage,             /**< Represents Apple Main Stage. */
        Ardour,                     /**< Represents Ardour. */
        AULab,                      /**< Represents AU Lab. */
        AUVal,                      /**< Represents Apple AU validator. */
        AvidProTools,               /**< Represents Avid Pro Tools. */
        BitwigStudio,               /**< Represents Bitwig Studio. */
        CakewalkSonar8,             /**< Represents Cakewalk Sonar 8. */
        CakewalkSonarGeneric,       /**< Represents Cakewalk Sonar. */
        CakewalkByBandlab,          /**< Represents Cakewalk by Bandlab. */
        DaVinciResolve,             /**< Represents DaVinci Resolve. */
        DigitalPerformer,           /**< Represents Digital Performer. */
        FinalCut,                   /**< Represents Apple Final Cut Pro. */
        FruityLoops,                /**< Represents Fruity Loops. */
        DRXPluginHost,             /**< Represents the DRX AudioPluginHost */
        MagixSamplitude,            /**< Represents Magix Samplitude. */
        MagixSequoia,               /**< Represents Magix Sequoia. */
        MergingPyramix,             /**< Represents Merging Pyramix. */
        MuseReceptorGeneric,        /**< Represents Muse Receptor. */
        Maschine,                   /**< Represents Native Instruments Maschine. */
        pluginval,                  /**< Represents pluginval. */
        Reaper,                     /**< Represents Cockos Reaper. */
        Reason,                     /**< Represents Reason. */
        Renoise,                    /**< Represents Renoise. */
        SADiE,                      /**< Represents SADiE. */
        SteinbergCubase4,           /**< Represents Steinberg Cubase 4. */
        SteinbergCubase5,           /**< Represents Steinberg Cubase 5. */
        SteinbergCubase5Bridged,    /**< Represents Steinberg Cubase 5 Bridged. */
        SteinbergCubase6,           /**< Represents Steinberg Cubase 6. */
        SteinbergCubase7,           /**< Represents Steinberg Cubase 7. */
        SteinbergCubase8,           /**< Represents Steinberg Cubase 8. */
        SteinbergCubase8_5,         /**< Represents Steinberg Cubase 8.5. */
        SteinbergCubase9,           /**< Represents Steinberg Cubase 9. */
        SteinbergCubase9_5,         /**< Represents Steinberg Cubase 9.5. */
        SteinbergCubase10,          /**< Represents Steinberg Cubase 10. */
        SteinbergCubase10_5,        /**< Represents Steinberg Cubase 10.5. */
        SteinbergCubaseGeneric,     /**< Represents Steinberg Cubase. */
        SteinbergNuendo3,           /**< Represents Steinberg Nuendo 3. */
        SteinbergNuendo4,           /**< Represents Steinberg Nuendo 4. */
        SteinbergNuendo5,           /**< Represents Steinberg Nuendo 5. */
        SteinbergNuendoGeneric,     /**< Represents Steinberg Nuendo. */
        SteinbergWavelab5,          /**< Represents Steinberg Wavelab 5. */
        SteinbergWavelab6,          /**< Represents Steinberg Wavelab 6. */
        SteinbergWavelab7,          /**< Represents Steinberg Wavelab 7. */
        SteinbergWavelab8,          /**< Represents Steinberg Wavelab 8. */
        SteinbergWavelabGeneric,    /**< Represents Steinberg Wavelab. */
        SteinbergTestHost,          /**< Represents Steinberg's VST3 Test Host. */
        StudioOne,                  /**< Represents PreSonus Studio One. */
        Tracktion3,                 /**< Represents Tracktion 3. */
        TracktionGeneric,           /**< Represents Tracktion. */
        TracktionWaveform,          /**< Represents Tracktion Waveform. */
        VBVSTScanner,               /**< Represents VB Audio VST Scanner. */
        ViennaEnsemblePro,          /**< Represents Vienna Ensemble Pro. */
        WaveBurner                  /**< Represents Apple WaveBurner. */
    };

    HostType type;

    //==============================================================================
    /** Возвращает true, если the host is any version of Ableton Live. */
    b8 isAbletonLive() const noexcept       { return type == AbletonLive6 || type == AbletonLive7 || type == AbletonLive8
                                                      || type == AbletonLive9 || type == AbletonLive10 || type == AbletonLive11
                                                      || type == AbletonLiveGeneric; }
    /** Возвращает true, если the host is Adobe Audition. */
    b8 isAdobeAudition() const noexcept     { return type == AdobeAudition; }
    /** Возвращает true, если the host is com.apple.audio.InfoHelper. */
    b8 isAppleInfoHelper() const noexcept   { return type == AppleInfoHelper; }
    /** Возвращает true, если the host is Ardour. */
    b8 isArdour() const noexcept            { return type == Ardour; }
    /** Возвращает true, если the host is AU Lab. */
    b8 isAULab() const noexcept             { return type == AULab; }
    /** Возвращает true, если the host is auval. */
    b8 isAUVal() const noexcept             { return type == AUVal; }
    /** Возвращает true, если the host is Bitwig Studio. */
    b8 isBitwigStudio() const noexcept      { return type == BitwigStudio; }
    /** Возвращает true, если the host is any version of Steinberg Cubase. */
    b8 isCubase() const noexcept            { return type == SteinbergCubase4 || type == SteinbergCubase5 || type == SteinbergCubase5Bridged || type == SteinbergCubase6
                                                      || type == SteinbergCubase7 || type == SteinbergCubase8 || type == SteinbergCubase8_5 || type == SteinbergCubase9
                                                      || type == SteinbergCubase9_5 || type == SteinbergCubase10 || type == SteinbergCubase10_5 || type == SteinbergCubaseGeneric; }
    /** Возвращает true, если the host is Steinberg Cubase 7 or later. */
    b8 isCubase7orLater() const noexcept    { return isCubase() && ! (type == SteinbergCubase4 || type == SteinbergCubase5 || type == SteinbergCubase6); }
    /** Возвращает true, если the host is Steinberg Cubase 5 Bridged. */
    b8 isCubaseBridged() const noexcept     { return type == SteinbergCubase5Bridged; }
    /** Возвращает true, если the host is DaVinci Resolve. */
    b8 isDaVinciResolve() const noexcept    { return type == DaVinciResolve; }
    /** Возвращает true, если the host is Digital Performer. */
    b8 isDigitalPerformer() const noexcept  { return type == DigitalPerformer; }
    /** Возвращает true, если the host is Apple Final Cut Pro. */
    b8 isFinalCut() const noexcept          { return type == FinalCut; }
    /** Возвращает true, если the host is Fruity Loops. */
    b8 isFruityLoops() const noexcept       { return type == FruityLoops; }
    /** Возвращает true, если the host is Apple GarageBand. */
    b8 isGarageBand() const noexcept        { return type == AppleGarageBand; }
    /** Возвращает true, если the host is the DRX AudioPluginHost */
    b8 isDRXPluginHost() const noexcept    { return type == DRXPluginHost; }
    /** Возвращает true, если the host is Apple Logic Pro. */
    b8 isLogic() const noexcept             { return type == AppleLogic; }
    /** Возвращает true, если the host is Apple MainStage. */
    b8 isMainStage() const noexcept         { return type == AppleMainStage; }
    /** Возвращает true, если the host is any version of Steinberg Nuendo. */
    b8 isNuendo() const noexcept            { return type == SteinbergNuendo3 || type == SteinbergNuendo4  || type == SteinbergNuendo5 ||  type == SteinbergNuendoGeneric; }
    /** Возвращает true, если the host is pluginval. */
    b8 isPluginval() const noexcept         { return type == pluginval; }
    /** Возвращает true, если the host is Adobe Premiere Pro. */
    b8 isPremiere() const noexcept          { return type == AdobePremierePro; }
    /** Возвращает true, если the host is Avid Pro Tools. */
    b8 isProTools() const noexcept          { return type == AvidProTools; }
    /** Возвращает true, если the host is Merging Pyramix. */
    b8 isPyramix() const noexcept           { return type == MergingPyramix; }
    /** Возвращает true, если the host is Muse Receptor. */
    b8 isReceptor() const noexcept          { return type == MuseReceptorGeneric; }
    /** Возвращает true, если the host is Cockos Reaper. */
    b8 isReaper() const noexcept            { return type == Reaper; }
    /** Возвращает true, если the host is Reason. */
    b8 isReason() const noexcept            { return type == Reason; }
    /** Возвращает true, если the host is Renoise. */
    b8 isRenoise() const noexcept           { return type == Renoise; }
    /** Возвращает true, если the host is SADiE. */
    b8 isSADiE() const noexcept             { return type == SADiE; }
    /** Возвращает true, если the host is Magix Samplitude. */
    b8 isSamplitude() const noexcept        { return type == MagixSamplitude; }
    /** Возвращает true, если the host is Magix Sequoia. */
    b8 isSequoia() const noexcept           { return type == MagixSequoia; }
    /** Возвращает true, если the host is any version of Cakewalk Sonar. */
    b8 isSonar() const noexcept             { return type == CakewalkSonar8 || type == CakewalkSonarGeneric || type == CakewalkByBandlab; }
    /** Возвращает true, если the host is Steinberg's VST3 Test Host. */
    b8 isSteinbergTestHost() const noexcept { return type == SteinbergTestHost; }
    /** Возвращает true, если the host is any product from Steinberg. */
    b8 isSteinberg() const noexcept         { return isCubase() || isNuendo() || isWavelab() || isSteinbergTestHost(); }
    /** Возвращает true, если the host is PreSonus Studio One. */
    b8 isStudioOne() const noexcept         { return type == StudioOne; }
    /** Возвращает true, если the host is any version of Tracktion. */
    b8 isTracktion() const noexcept         { return type == Tracktion3 || type == TracktionGeneric || isTracktionWaveform(); }
    /** Возвращает true, если the host is Tracktion Waveform. */
    b8 isTracktionWaveform() const noexcept { return type == TracktionWaveform; }
    /** Возвращает true, если the host is VB Audio VST Scanner. */
    b8 isVBVSTScanner() const noexcept      { return type == VBVSTScanner; }
    /** Возвращает true, если the host is Vienna Ensemble Pro. */
    b8 isViennaEnsemblePro() const noexcept { return type == ViennaEnsemblePro; }
    /** Возвращает true, если the host is Apple WaveBurner. */
    b8 isWaveBurner() const noexcept        { return type == WaveBurner; }
    /** Возвращает true, если the host is any version of Steinberg WaveLab. */
    b8 isWavelab() const noexcept           { return isWavelabLegacy() || type == SteinbergWavelab7 || type == SteinbergWavelab8 || type == SteinbergWavelabGeneric; }
    /** Возвращает true, если the host is Steinberg WaveLab 6 or below. */
    b8 isWavelabLegacy() const noexcept     { return type == SteinbergWavelab5 || type == SteinbergWavelab6; }
    /** Возвращает true, если the host is Native Instruments Maschine. */
    b8 isMaschine() const noexcept          { return type == Maschine; }

    //==============================================================================
    /** Returns a human-readable description of the host. */
    tukk getHostDescription() const noexcept;

    //==============================================================================
    /** Возвращает true, если the plugin is connected with Inter-App Audio on iOS. */
    b8 isInterAppAudioConnected() const;
    /** Switches to the host application when Inter-App Audio is used on iOS. */
    z0 switchToHostApplication() const;
    /** Gets the host app's icon when Inter-App Audio is used on iOS. */
    Image getHostIcon (i32 size) const;

    //==============================================================================
    /** Returns the complete absolute path of the host application executable. */
    static Txt getHostPath()
    {
        return File::getSpecialLocation (File::hostApplicationPath).getFullPathName();
    }

    //==============================================================================
    /**
         Returns the plug-in format via which the plug-in file was loaded. This value is
         identical to AudioProcessor::wrapperType of the main audio processor of this
         plug-in. This function is useful for code that does not have access to the
         plug-in's main audio processor.

         @see AudioProcessor::wrapperType
    */
    static AudioProcessor::WrapperType getPluginLoadedAs() noexcept    { return jucePlugInClientCurrentWrapperType; }

    /** Возвращает true, если the AudioProcessor instance is an AAX plug-in running in AudioSuite. */
    static b8 isInAAXAudioSuite (AudioProcessor&);

    //==============================================================================

  #ifndef DOXYGEN
    // @internal
    static AudioProcessor::WrapperType jucePlugInClientCurrentWrapperType;
    static std::function<b8 (AudioProcessor&)> jucePlugInIsRunningInAudioSuiteFn;
    static Txt hostIdReportedByWrapper;
  #endif

private:
    static HostType getHostType();
};

} // namespace drx
