#include "OptionsConfig.h"

//---------------------------------------------------------------------
// Define the common options mapping.
// These options appear on the Common tab but are only a shortcut to update
// the real settings in another section. For example, toggling "bEnableBorderRegion"
// on the Common tab will update the key "bEnableBorderRegion" in the MAIN section.
const CommonOption commonOptions[] = {
    { L"bEnableBorderRegion", L"MAIN", L"bEnableBorderRegion" },
    // The following two require custom handling (e.g. splitting a resolution string)
    // in your OptionsWindow code. For example, selecting a resolution in the drop?down
    // should update both iSize W and iSize H under the Display section.
    { L"Resolution", L"Display", L"Resolution" },
    { L"AntiAliasing", L"Display", L"iMultiSample" }
};
const int numCommonOptions = sizeof(commonOptions) / sizeof(commonOptions[0]);

//---------------------------------------------------------------------
// For building the UI, we need an array of option names for the Common tab.
// (These are the keys shown in the Common tab; note that they are not written directly
// to the INI file.)
static const wchar_t* commonOpts[] = {
    L"bEnableBorderRegion",
    L"Resolution",
    L"AntiAliasing"
};

//---------------------------------------------------------------------
// Define the option arrays for each INI section.
// (Only “useful” options are included; you may modify these arrays as needed.)
static const wchar_t* generalOpts[] = {
    L"SStartingCell", L"SStartingCellY", L"SStartingCellX", L"SStartingWorld",
    L"STestFile10", L"STestFile9", L"STestFile8", L"STestFile7", L"STestFile6",
    L"STestFile5", L"STestFile4", L"STestFile3", L"STestFile2", L"STestFile1",
    L"bEnableProfile", L"bDrawSpellContact", L"bRunMiddleLowLevelProcess",
    L"iHoursToSleep", L"bActorLookWithHavok", L"SMainMenuMusicTrack",
    L"bUseEyeEnvMapping", L"bFixFaceNormals", L"bUseFaceGenHeads", L"bFaceMipMaps",
    L"bFaceGenTexturing", L"bDefaultCOCPlacement", L"uGridDistantTreeRange",
    L"uGridDistantCount", L"uGridsToLoad", L"fGlobalTimeMultiplier", L"bNewAnimation",
    L"fAnimationDefaultBlend", L"fAnimationMult", L"bFixAIPackagesOnLoad",
    L"bForceReloadOnEssentialCharacterDeath", L"bKeepPluginWhenMerging",
    L"bCreate Maps Enable", L"SLocalSavePath", L"SLocalMasterPath",
    L"bDisableDuplicateReferenceCheck", L"bTintMipMaps", L"uInterior Cell Buffer",
    L"uExterior Cell Buffer", L"iIntroSequencePriority", L"bPreloadIntroSequence",
    L"fStaticScreenWaitTime", L"SCreditsMenuMovie", L"SMainMenuMovie",
    L"SMainMenuMovieIntro", L"SIntroSequence", L"iFPSClamp", L"bRunVTuneTest",
    L"STestFile1", L"bActivateAllQuestScripts", L"fQuestScriptDelayTime",
    L"SMainMenuMusic", L"bUseThreadedBlood", L"bUseThreadedMorpher",
    L"bExternalLODDataFiles"
};

static const wchar_t* displayOpts[] = {
    L"uVideoDeviceIdentifierPart1", L"uVideoDeviceIdentifierPart2",
    L"uVideoDeviceIdentifierPart3", L"uVideoDeviceIdentifierPart4",
    L"fDecalLifetime", L"bEquippedTorchesCastShadows", L"bReportBadTangentSpace",
    L"bStaticMenuBackground", L"bForcePow2Textures", L"bForce1XShaders",
    L"bHighQuality20Lighting", L"bAllow20HairShader", L"bAllowScreenShot",
    L"iMultiSample", L"bDoTallGrassEffect", L"bForceMultiPass", L"bDoTexturePass",
    L"bDoSpecularPass", L"bDoDiffusePass", L"bDoAmbientPass", L"bDoCanopyShadowPass",
    L"bDrawShadows", L"bUseRefractionShader", L"bUse Shaders", L"iNPatchNOrder",
    L"iNPatchPOrder", L"iNPatches", L"iLocation Y", L"iLocation X", L"bFull Screen",
    L"iSize W", L"iSize H", L"iAdapter", L"iScreenShotIndex", L"SScreenShotBaseName",
    L"iAutoViewMinDistance", L"iAutoViewHiFrameRate", L"iAutoViewLowFrameRate",
    L"bAutoViewDistance", L"fDefaultFOV", L"fNearDistance", L"fFarDistance",
    L"iDebugTextLeftRightOffset", L"iDebugTextTopBottomOffset", L"bShowMenuTextureUse",
    L"iDebugText", L"bLocalMapShader", L"bDoImageSpaceEffects", L"fShadowLOD2",
    L"fShadowLOD1", L"fLightLOD2", L"fLightLOD1", L"fSpecularLOD2", L"fSpecularLOD1",
    L"fEnvMapLOD2", L"fEnvMapLOD1", L"fEyeEnvMapLOD2", L"fEyeEnvMapLOD1",
    L"iPresentInterval", L"iShadowFilter", L"iActorShadowCountExt",
    L"iActorShadowCountInt", L"bActorSelfShadowing", L"bShadowsOnGrass",
    L"bDynamicWindowReflections", L"iTexMipMapSkip", L"fGrassStartFadeDistance",
    L"fGrassEndDistance", L"bDecalsOnSkinnedGeometry", L"bFullBrightLighting",
    L"iMaxLandscapeTextures", L"bLODPopActors", L"bLODPopItems", L"bLODPopObjects"
};

static const wchar_t* controlsOpts[] = {
    L"fVersion", L"Forward", L"Back", L"Slide Left", L"Slide Right", L"Use",
    L"Activate", L"Block", L"Cast", L"Ready Item", L"Crouch/Sneak", L"Run",
    L"Always Run", L"Auto Move", L"Jump", L"Toggle POV", L"Menu Mode", L"Rest",
    L"Quick Menu", L"Quick1", L"Quick2", L"Quick3", L"Quick4", L"Quick5",
    L"Quick6", L"Quick7", L"Quick8", L"QuickSave", L"QuickLoad", L"Grab",
    L"bInvertYValues", L"fXenonLookXYMult", L"fMouseSensitivity",
    L"iJoystickMoveFrontBack", L"iJoystickMoveLeftRight", L"fJoystickMoveFBMult",
    L"fJoystickMoveLRMult", L"iJoystickLookUpDown", L"iJoystickLookLeftRight",
    L"fJoystickLookUDMult", L"fJoystickLookLRMult", L"fXenonMenuMouseXYMult",
    L"bBackground Mouse", L"bBackground Keyboard", L"bUse Joystick",
    L"fXenonLookMult", L"fXenonMenuStickSpeedMaxMod", L"iXenonMenuStickSpeedThreshold",
    L"iXenonMenuStickThreshold", L"iLanguage"
};

static const wchar_t* waterOpts[] = {
    L"fAlpha", L"uSurfaceTextureSize", L"SSurfaceTexture",
    L"SNearWaterOutdoorID", L"SNearWaterIndoorID",
    L"fNearWaterOutdoorTolerance", L"fNearWaterIndoorTolerance",
    L"fNearWaterUnderwaterVolume", L"fNearWaterUnderwaterFreq",
    L"uNearWaterPoints", L"uNearWaterRadius", L"uSurfaceFrameCount",
    L"uSurfaceFPS", L"bUseWaterReflectionsMisc", L"bUseWaterReflectionsStatics",
    L"bUseWaterReflectionsTrees", L"bUseWaterReflectionsActors",
    L"bUseWaterReflections", L"bUseWaterHiRes", L"bUseWaterDisplacements",
    L"bUseWaterShader", L"uDepthRange", L"bUseWaterDepth", L"bUseWaterLOD",
    L"fTileTextureDivisor", L"fSurfaceTileSize"
};

static const wchar_t* audioOpts[] = {
    L"bDSoundHWAcceleration", L"fMinSoundVel", L"fMetalLargeMassMin",
    L"fMetalMediumMassMin", L"fStoneLargeMassMin", L"fStoneMediumMassMin",
    L"fWoodLargeMassMin", L"fWoodMediumMassMin", L"fDialogAttenuationMax",
    L"fDialogAttenuationMin", L"bUseSoundDebugInfo", L"fUnderwaterFrequencyDelta",
    L"bUseSoftwareAudio3D", L"fDefaultEffectsVolume", L"fDefaultMusicVolume",
    L"fDefaultFootVolume", L"fDefaultVoiceVolume", L"fDefaultMasterVolume",
    L"bMusicEnabled", L"bSoundEnabled", L"fLargeWeaponWeightMin",
    L"fMediumWeaponWeightMin", L"fSkinLargeMassMin", L"fSkinMediumMassMin",
    L"fChainLargeMassMin", L"fChainMediumMassMin", L"fDBVoiceAttenuationIn2D",
    L"iCollisionSoundTimeDelta", L"fGlassLargeMassMin", L"fGlassMediumMassMin",
    L"fClothLargeMassMin", L"fClothMediumMassMin", L"fEarthLargeMassMin",
    L"fEarthMediumMassMin", L"bUseSpeedForWeaponSwish", L"fLargeWeaponSpeedMax",
    L"fMediumWeaponSpeedMax", L"fPlayerFootVolume", L"fDSoundRolloffFactor",
    L"fMaxFootstepDistance", L"fHeadroomdB", L"iMaxImpactSoundCount",
    L"fMainMenuMusicVolume"
};

static const wchar_t* shockBoltOpts[] = {
    L"bDebug", L"fGlowColorB", L"fGlowColorG", L"fGlowColorR",
    L"fCoreColorB", L"fCoreColorG", L"fCoreColorR", L"fCastVOffset",
    L"iNumBolts", L"fBoltGrowWidth", L"fBoltSmallWidth",
    L"fTortuosityVariance", L"fSegmentVariance", L"fBoltsRadius"
};

static const wchar_t* pathfindingOpts[] = {
    L"bDrawPathsDefault", L"bPathMovementOnly", L"bDrawSmoothFailures",
    L"bDebugSmoothing", L"bSmoothPaths", L"bSnapToAngle",
    L"bDebugAvoidance", L"bDisableAvoidance", L"bBackgroundPathing"
};

static const wchar_t* mainOpts[] = {
    L"bEnableBorderRegion"
};

static const wchar_t* combatOpts[] = {
    L"bEnableBowZoom", L"bDebugCombatAvoidance", L"fMinBloodDamage",
    L"fHitVectorDelay", L"iShowHitVector"
};

static const wchar_t* havokOpts[] = {
    L"bDisablePlayerCollision", L"fJumpAnimDelay", L"bTreeTops", L"iSimType",
    L"bPreventHavokAddAll", L"bPreventHavokAddClutter", L"fMaxTime",
    L"bHavokDebug", L"fRF", L"fOD", L"fSE", L"fSD", L"iResetCounter",
    L"fMoveLimitMass", L"iUpdateType", L"bHavokPick"
};

static const wchar_t* interfaceOpts[] = {
    L"fDlgLookMult", L"fDlgLookAdj", L"fDlgLookDegStop", L"fDlgLookDegStart",
    L"fDlgFocus", L"fKeyRepeatInterval", L"fKeyRepeatTime",
    L"fActivatePickSphereRadius", L"fMenuModeAnimBlend", L"iSafeZoneX",
    L"iSafeZoneY", L"iSafeZoneXWide", L"iSafeZoneYWide"
};

static const wchar_t* loadingBarOpts[1] = { nullptr };

static const wchar_t* menuOpts[] = {
    L"fCreditsScrollSpeed", L"iConsoleTextYPos", L"iConsoleTextXPos",
    L"iConsoleVisibleLines", L"iConsoleHistorySize", L"rDebugTextColor",
    L"iConsoleFont", L"iDebugTextFont"
};

static const wchar_t* gameplayOpts[] = {
    L"bDisableDynamicCrosshair", L"bSaveOnTravel", L"bSaveOnWait",
    L"bSaveOnRest", L"bCrossHair", L"iDifficultyLevel", L"bGeneralSubtitles",
    L"bDialogueSubtitles", L"bInstantLevelUp", L"bHealthBarShowing",
    L"fHealthBarFadeOutSpeed", L"fHealthBarSpeed", L"fHealthBarHeight",
    L"fHealthBarWidth", L"fHealthBarEmittanceFadeTime", L"fHealthBarEmittanceTime"
};

static const wchar_t* fontsOpts[] = {
    L"SFontFile_1", L"SFontFile_2", L"SFontFile_3", L"SFontFile_4", L"SFontFile_5"
};

static const wchar_t* speedTreeOpts[] = {
    L"iTreeClonesAllowed", L"fCanopyShadowGrassMult", L"iCanopyShadowScale",
    L"fTreeForceMaxBudAngle", L"fTreeForceMinBudAngle", L"fTreeForceLeafDimming",
    L"fTreeForceBranchDimming", L"fTreeForceCS", L"fTreeForceLLA",
    L"fTreeLODExponent", L"bEnableTrees", L"bForceFullLOD"
};

static const wchar_t* debugOpts[] = {
    L"bDebugFaceGenCriticalSection", L"bDebugFaceGenMultithreading",
    L"bDebugSaveBuffer"
};

static const wchar_t* backgroundLoadOpts[1] = { nullptr };

static const wchar_t* lodOpts[] = {
    L"fLodDistance", L"bUseFaceGenLOD", L"iLODTextureTiling",
    L"iLODTextureSizePow2", L"fLODNormalTextureBlend", L"bDisplayLODLand",
    L"bDisplayLODBuildings", L"bDisplayLODTrees", L"bLODPopTrees",
    L"bLODPopActors", L"bLODPopItems", L"bLODPopObjects", L"fLODFadeOutMultActors",
    L"fLODFadeOutMultItems", L"fLODFadeOutMultObjects", L"fLODMultLandscape",
    L"fLODMultTrees", L"fLODMultActors", L"fLODMultItems", L"fLODMultObjects",
    L"iFadeNodeMinNearDistance", L"fLODFadeOutPercent", L"fLODBoundRadiusMult",
    L"fTalkingDistance", L"fTreeLODMax", L"fTreeLODMin", L"fTreeLODDefault"
};

static const wchar_t* weatherOpts[] = {
    L"fSunGlareSize", L"fSunBaseSize", L"bPrecipitation", L"fAlphaReduce",
    L"SBumpFadeColor", L"SLerpCloseColor", L"SEnvReduceColor"
};

static const wchar_t* voiceOpts[] = {
    L"SFileTypeLTF", L"SFileTypeLip", L"SFileTypeSource", L"SFileTypeGame"
};

static const wchar_t* grassOpts[] = {
    L"iMinGrassSize", L"fGrassEndDistance", L"fGrassStartFadeDistance",
    L"bGrassPointLighting", L"bDrawShaderGrass", L"iGrassDensityEvalSize",
    L"iMaxGrassTypesPerTexure", L"fWaveOffsetRange"
};

static const wchar_t* landscapeOpts[] = {
    L"bCurrentCellOnly", L"bPreventSafetyCheck", L"fLandTextureTilingMult"
};

static const wchar_t* lightAttenOpts[] = {
    L"fQuadraticRadiusMult", L"fLinearRadiusMult", L"bOutQuadInLin",
    L"fConstantValue", L"fQuadraticValue", L"fLinearValue", L"uQuadraticMethod",
    L"uLinearMethod", L"fFlickerMovement", L"bUseQuadratic", L"bUseLinear",
    L"bUseConstant"
};

static const wchar_t* blurHDRInterOpts[] = {
    L"fTargetLUM", L"fUpperLUMClamp", L"fEmissiveHDRMult", L"fEyeAdaptSpeed",
    L"fBrightScale", L"fBrightClamp", L"fBlurRadius", L"iNumBlurpasses"
};

static const wchar_t* blurHDROpts[] = {
    L"fTargetLUM", L"fUpperLUMClamp", L"fGrassDimmer", L"fTreeDimmer",
    L"fEmissiveHDRMult", L"fEyeAdaptSpeed", L"fSunlightDimmer", L"fSIEmmisiveMult",
    L"fSISpecularMult", L"fSkyBrightness", L"fSunBrightness", L"fBrightScale",
    L"fBrightClamp", L"fBlurRadius", L"iNumBlurpasses", L"iBlendType",
    L"bDoHighDynamicRange"
};

static const wchar_t* blurOpts[] = {
    L"fSunlightDimmer", L"fSIEmmisiveMult", L"fSISpecularMult",
    L"fSkyBrightness", L"fSunBrightness", L"fAlphaAddExterior",
    L"fAlphaAddInterior", L"iBlurTexSize", L"fBlurRadius", L"iNumBlurpasses",
    L"iBlendType", L"bUseBlurShader"
};

static const wchar_t* gethitOpts[] = {
    L"fBlurAmmount", L"fBlockedTexOffset", L"fHitTexOffset"
};

static const wchar_t* messagesOpts[] = {
    L"bBlockMessageBoxes", L"bSkipProgramFlows", L"bAllowYesToAll",
    L"bDisableWarning", L"iFileLogging"
};

static const wchar_t* distantLodOpts[] = {
    L"bUseLODLandData", L"fFadeDistance"
};

static const wchar_t* genWarningsOpts[] = {
    L"SGeneralMasterMismatchWarning", L"SMasterMismatchWarning"
};

static const wchar_t* archiveOpts[] = {
    L"SMasterMiscArchiveFileName", L"SMasterVoicesArchiveFileName2",
    L"SMasterVoicesArchiveFileName1", L"SMasterSoundsArchiveFileName",
    L"SMasterTexturesArchiveFileName1", L"SMasterMeshesArchiveFileName",
    L"SInvalidationFile", L"iRetainFilenameOffsetTable", L"iRetainFilenameStringTable",
    L"iRetainDirectoryStringTable", L"bCheckRuntimeCollisions",
    L"bInvalidateOlderFiles", L"bUseArchives"
};

static const wchar_t* cameraPathOpts[] = {
    L"iTake", L"SDirectoryName", L"iFPS", L"SNif"
};

static const wchar_t* absorbOpts[] = {
    L"fAbsorbGlowColorB", L"fAbsorbGlowColorG", L"fAbsorbGlowColorR",
    L"fAbsorbCoreColorB", L"fAbsorbCoreColorG", L"fAbsorbCoreColorR",
    L"iAbsorbNumBolts", L"fAbsorbBoltGrowWidth", L"fAbsorbBoltSmallWidth",
    L"fAbsorbTortuosityVariance", L"fAbsorbSegmentVariance", L"fAbsorbBoltsRadius"
};

static const wchar_t* openmpOpts[] = {
    L"iThreads", L"iOpenMPLevel"
};

static const wchar_t* testAllCellsOpts[] = {
    L"bFileShowTextures", L"bFileShowIcons", L"bFileSkipIconChecks",
    L"bFileTestLoad", L"bFileNeededMessage", L"bFileGoneMessage",
    L"bFileSkipModelChecks"
};

static const wchar_t* copyProtectionOpts[] = {
    L"SCopyProtectionMessage2", L"SCopyProtectionTitle2",
    L"SCopyProtectionMessage", L"SCopyProtectionTitle"
};

//---------------------------------------------------------------------
// Now build the array of INI sections.
// Note: The first section is "Common" which is used only in the Options window;
// it is not written to the INI file.
const IniSection iniSections[] = {
//    { L"Common", commonOpts, sizeof(commonOpts) / sizeof(commonOpts[0]) },
    { L"General", generalOpts, sizeof(generalOpts) / sizeof(generalOpts[0]) },
    { L"Display", displayOpts, sizeof(displayOpts) / sizeof(displayOpts[0]) },
    { L"Controls", controlsOpts, sizeof(controlsOpts) / sizeof(controlsOpts[0]) },
    { L"Water", waterOpts, sizeof(waterOpts) / sizeof(waterOpts[0]) },
    { L"Audio", audioOpts, sizeof(audioOpts) / sizeof(audioOpts[0]) },
    { L"ShockBolt", shockBoltOpts, sizeof(shockBoltOpts) / sizeof(shockBoltOpts[0]) },
    { L"Pathfinding", pathfindingOpts, sizeof(pathfindingOpts) / sizeof(pathfindingOpts[0]) },
    { L"MAIN", mainOpts, sizeof(mainOpts) / sizeof(mainOpts[0]) },
    { L"Combat", combatOpts, sizeof(combatOpts) / sizeof(combatOpts[0]) },
    { L"HAVOK", havokOpts, sizeof(havokOpts) / sizeof(havokOpts[0]) },
    { L"Interface", interfaceOpts, sizeof(interfaceOpts) / sizeof(interfaceOpts[0]) },
    { L"LoadingBar", loadingBarOpts, 0 },
    { L"Menu", menuOpts, sizeof(menuOpts) / sizeof(menuOpts[0]) },
    { L"GamePlay", gameplayOpts, sizeof(gameplayOpts) / sizeof(gameplayOpts[0]) },
    { L"Fonts", fontsOpts, sizeof(fontsOpts) / sizeof(fontsOpts[0]) },
    { L"SpeedTree", speedTreeOpts, sizeof(speedTreeOpts) / sizeof(speedTreeOpts[0]) },
    { L"Debug", debugOpts, sizeof(debugOpts) / sizeof(debugOpts[0]) },
    { L"BackgroundLoad", backgroundLoadOpts, 0 },
    { L"LOD", lodOpts, sizeof(lodOpts) / sizeof(lodOpts[0]) },
    { L"Weather", weatherOpts, sizeof(weatherOpts) / sizeof(weatherOpts[0]) },
    { L"Voice", voiceOpts, sizeof(voiceOpts) / sizeof(voiceOpts[0]) },
    { L"Grass", grassOpts, sizeof(grassOpts) / sizeof(grassOpts[0]) },
    { L"Landscape", landscapeOpts, sizeof(landscapeOpts) / sizeof(landscapeOpts[0]) },
    { L"bLightAttenuation", lightAttenOpts, sizeof(lightAttenOpts) / sizeof(lightAttenOpts[0]) },
    { L"BlurShaderHDRInterior", blurHDRInterOpts, sizeof(blurHDRInterOpts) / sizeof(blurHDRInterOpts[0]) },
    { L"BlurShaderHDR", blurHDROpts, sizeof(blurHDROpts) / sizeof(blurHDROpts[0]) },
    { L"BlurShader", blurOpts, sizeof(blurOpts) / sizeof(blurOpts[0]) },
    { L"GethitShader", gethitOpts, sizeof(gethitOpts) / sizeof(gethitOpts[0]) },
    { L"MESSAGES", messagesOpts, sizeof(messagesOpts) / sizeof(messagesOpts[0]) },
    { L"DistantLOD", distantLodOpts, sizeof(distantLodOpts) / sizeof(distantLodOpts[0]) },
    { L"GeneralWarnings", genWarningsOpts, sizeof(genWarningsOpts) / sizeof(genWarningsOpts[0]) },
    { L"Archive", archiveOpts, sizeof(archiveOpts) / sizeof(archiveOpts[0]) },
    { L"CameraPath", cameraPathOpts, sizeof(cameraPathOpts) / sizeof(cameraPathOpts[0]) },
    { L"Absorb", absorbOpts, sizeof(absorbOpts) / sizeof(absorbOpts[0]) },
    { L"OPENMP", openmpOpts, sizeof(openmpOpts) / sizeof(openmpOpts[0]) },
    { L"TestAllCells", testAllCellsOpts, sizeof(testAllCellsOpts) / sizeof(testAllCellsOpts[0]) },
    { L"CopyProtectionStrings", copyProtectionOpts, sizeof(copyProtectionOpts) / sizeof(copyProtectionOpts[0]) }
};
const int numIniSections = sizeof(iniSections) / sizeof(iniSections[0]);
