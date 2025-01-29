#pragma once

#ifdef NANI_USE_TRACY

#include <tracy/Tracy.hpp>

#define NANI_TracyNoop TracyNoop

#define NANI_ZoneNamed(x, y) ZoneNamed(x, y)
#define NANI_ZoneNamedN(x, y, z) ZoneNamedN(x, y, z)
#define NANI_ZoneNamedC(x, y, z) ZoneNamedC(x, y, z)
#define NANI_ZoneNamedNC(x, y, z, w) ZoneNamedNC(x, y, z, w)

#define NANI_ZoneTransient(x, y) ZoneTransient(x, y)
#define NANI_ZoneTransientN(x, y, z) ZoneTransientN(x, y, z)

#define NANI_ZoneScoped ZoneScoped
#define NANI_ZoneScopedN(x) ZoneScopedN(x)
#define NANI_ZoneScopedC(x) ZoneScopedC(x)
#define NANI_ZoneScopedNC(x, y) ZoneScopedNC(x, y)

#define NANI_ZoneText(x, y) ZoneText(x, y)
#define NANI_ZoneTextV(x, y, z) ZoneTextV(x, y, z)
#define NANI_ZoneTextF(x, ...) ZoneTextF(x, __VA_ARGS__)
#define NANI_ZoneTextVF(x, y, ...) ZoneTextVF(x, y, __VA_ARGS__)
#define NANI_ZoneName(x, y) ZoneName(x, y)
#define NANI_ZoneNameV(x, y, z) ZoneNameV(x, y, z)
#define NANI_ZoneNameF(x, ...) ZoneNameF(x, __VA_ARGS__)
#define NANI_ZoneNameVF(x, y, ...) ZoneNameVF(x, y, __VA_ARGS__)
#define NANI_ZoneColor(x) ZoneColor(x)
#define NANI_ZoneColorV(x, y) ZoneColorV(x, y)
#define NANI_ZoneValue(x) ZoneValue(x)
#define NANI_ZoneValueV(x, y) ZoneValueV(x, y)
#define NANI_ZoneIsActive ZoneIsActive
#define NANI_ZoneIsActiveV(x) ZoneIsActiveV(x)

#define NANI_FrameMark FrameMark
#define NANI_FrameMarkNamed(x) FrameMarkNamed(x)
#define NANI_FrameMarkStart(x) FrameMarkStart(x)
#define NANI_FrameMarkEnd(x) FrameMarkEnd(x)

#define NANI_FrameImage(x, y, z, w, a) FrameImage(x, y, z, w, a)

#define NANI_TracyLockable(type, varname) TracyLockable(type, varname)
#define NANI_TracyLockableN(type, varname, desc) TracyLockableN(type, varname, desc)
#define NANI_TracySharedLockable(type, varname) TracySharedLockable(type, varname)
#define NANI_TracySharedLockableN(type, varname, desc) TracySharedLockableN(type, varname, desc)
#define NANI_LockableBase(type) LockableBase(type)
#define NANI_SharedLockableBase(type) SharedLockableBase(type)
#define NANI_LockMark(x) LockMark(x)
#define NANI_LockableName(x, y, z) LockableName(x, y, z)

#define NANI_TracyPlot(x, y) TracyPlot(x, y)
#define NANI_TracyPlotConfig(x, y, z, w, a) TracyPlotConfig(x, y, z, w, a)

#define NANI_TracyMessage(x, y) TracyMessage(x, y)
#define NANI_TracyMessageL(x) TracyMessageL(x)
#define NANI_TracyMessageC(x, y, z) TracyMessageC(x, y, z)
#define NANI_TracyMessageLC(x, y) TracyMessageLC(x, y)
#define NANI_TracyAppInfo(x, y) TracyAppInfo(x, y)

#define NANI_TracyAlloc(x, y) TracyAlloc(x, y)
#define NANI_TracyFree(x) TracyFree(x)
#define NANI_TracyMemoryDiscard(x) TracyMemoryDiscard(x)
#define NANI_TracySecureAlloc(x, y) TracySecureAlloc(x, y)
#define NANI_TracySecureFree(x) TracySecureFree(x)
#define NANI_TracySecureMemoryDiscard(x) TracySecureMemoryDiscard(x)

#define NANI_TracyAllocN(x, y, z) TracyAllocN(x, y, z)
#define NANI_TracyFreeN(x, y) TracyFreeN(x, y)
#define NANI_TracySecureAllocN(x, y, z) TracySecureAllocN(x, y, z)
#define NANI_TracySecureFreeN(x, y) TracySecureFreeN(x, y)

#define NANI_ZoneNamedS(x, y, z) ZoneNamedS(x, y, z)
#define NANI_ZoneNamedNS(x, y, z, w) ZoneNamedNS(x, y, z, w)
#define NANI_ZoneNamedCS(x, y, z, w) ZoneNamedCS(x, y, z, w)
#define NANI_ZoneNamedNCS(x, y, z, w, a) ZoneNamedNCS(x, y, z, w, a)

#define NANI_ZoneTransientS(x, y, z) ZoneTransientS(x, y, z)
#define NANI_ZoneTransientNS(x, y, z, w) ZoneTransientNS(x, y, z, w)

#define NANI_ZoneScopedS(x) ZoneScopedS(x)
#define NANI_ZoneScopedNS(x, y) ZoneScopedNS(x, y)
#define NANI_ZoneScopedCS(x, y) ZoneScopedCS(x, y)
#define NANI_ZoneScopedNCS(x, y, z) ZoneScopedNCS(x, y, z)

#define NANI_TracyAllocS(x, y, z) TracyAllocS(x, y, z)
#define NANI_TracyFreeS(x, y) TracyFreeS(x, y)
#define NANI_TracyMemoryDiscardS(x, y) TracyMemoryDiscardS(x, y)
#define NANI_TracySecureAllocS(x, y, z) TracySecureAllocS(x, y, z)
#define NANI_TracySecureFreeS(x, y) TracySecureFreeS(x, y)
#define NANI_TracySecureMemoryDiscardS(x, y) TracySecureMemoryDiscardS(x, y)

#define NANI_TracyAllocNS(x, y, z, w) TracyAllocNS(x, y, z, w)
#define NANI_TracyFreeNS(x, y, z) TracyFreeNS(x, y, z)
#define NANI_TracySecureAllocNS(x, y, z, w) TracySecureAllocNS(x, y, z, w)
#define NANI_TracySecureFreeNS(x, y, z) TracySecureFreeNS(x, y, z)

#define NANI_TracyMessageS(x, y, z) TracyMessageS(x, y, z)
#define NANI_TracyMessageLS(x, y) TracyMessageLS(x, y)
#define NANI_TracyMessageCS(x, y, z, w) TracyMessageCS(x, y, z, w)
#define NANI_TracyMessageLCS(x, y, z) TracyMessageLCS(x, y, z)

#define NANI_TracySourceCallbackRegister(x, y) TracySourceCallbackRegister(x, y)
#define NANI_TracyParameterRegister(x, y) TracyParameterRegister(x, y)
#define NANI_TracyParameterSetup(x, y, z, w) TracyParameterSetup(x, y, z, w)
#define NANI_TracyIsConnected TracyIsConnected
#define NANI_TracyIsStarted TracyIsStarted
#define NANI_TracySetProgramName(x) TracySetProgramName(x)

#define NANI_TracyFiberEnter(x) TracyFiberEnter(x)
#define NANI_TracyFiberEnterHint(x, y) TracyFiberEnterHint(x, y)
#define NANI_TracyFiberLeave TracyFiberLeave

#else

#define NANI_ZoneNamed(x, y)
#define NANI_ZoneNamedN(x, y, z)
#define NANI_ZoneNamedC(x, y, z)
#define NANI_ZoneNamedNC(x, y, z, w)

#define NANI_ZoneTransient(x, y)
#define NANI_ZoneTransientN(x, y, z)

#define NANI_ZoneScoped
#define NANI_ZoneScopedN(x)
#define NANI_ZoneScopedC(x)
#define NANI_ZoneScopedNC(x, y)

#define NANI_ZoneText(x, y)
#define NANI_ZoneTextV(x, y, z)
#define NANI_ZoneTextF(x, ...)
#define NANI_ZoneTextVF(x, y, ...)
#define NANI_ZoneName(x, y)
#define NANI_ZoneNameV(x, y, z)
#define NANI_ZoneNameF(x, ...)
#define NANI_ZoneNameVF(x, y, ...)
#define NANI_ZoneColor(x)
#define NANI_ZoneColorV(x, y)
#define NANI_ZoneValue(x)
#define NANI_ZoneValueV(x, y)
#define NANI_ZoneIsActive false
#define NANI_ZoneIsActiveV(x) false

#define NANI_FrameMark
#define NANI_FrameMarkNamed(x)
#define NANI_FrameMarkStart(x)
#define NANI_FrameMarkEnd(x)

#define NANI_FrameImage(x, y, z, w, a)

#define NANI_TracyLockable(type, varname) type varname
#define NANI_TracyLockableN(type, varname, desc) type varname
#define NANI_TracySharedLockable(type, varname) type varname
#define NANI_TracySharedLockableN(type, varname, desc) type varname
#define NANI_LockableBase(type) type
#define NANI_SharedLockableBase(type) type
#define NANI_LockMark(x)
#define NANI_LockableName(x, y, z)

#define NANI_TracyPlot(x, y)
#define NANI_TracyPlotConfig(x, y, z, w, a)

#define NANI_TracyMessage(x, y)
#define NANI_TracyMessageL(x)
#define NANI_TracyMessageC(x, y, z)
#define NANI_TracyMessageLC(x, y)
#define NANI_TracyAppInfo(x, y)

#define NANI_TracyAlloc(x, y)
#define NANI_TracyFree(x)
#define NANI_TracyMemoryDiscard(x)
#define NANI_TracySecureAlloc(x, y)
#define NANI_TracySecureFree(x)
#define NANI_TracySecureMemoryDiscard(x)

#define NANI_TracyAllocN(x, y, z)
#define NANI_TracyFreeN(x, y)
#define NANI_TracySecureAllocN(x, y, z)
#define NANI_TracySecureFreeN(x, y)

#define NANI_ZoneNamedS(x, y, z)
#define NANI_ZoneNamedNS(x, y, z, w)
#define NANI_ZoneNamedCS(x, y, z, w)
#define NANI_ZoneNamedNCS(x, y, z, w, a)

#define NANI_ZoneTransientS(x, y, z)
#define NANI_ZoneTransientNS(x, y, z, w)

#define NANI_ZoneScopedS(x)
#define NANI_ZoneScopedNS(x, y)
#define NANI_ZoneScopedCS(x, y)
#define NANI_ZoneScopedNCS(x, y, z)

#define NANI_TracyAllocS(x, y, z)
#define NANI_TracyFreeS(x, y)
#define NANI_TracyMemoryDiscardS(x, y)
#define NANI_TracySecureAllocS(x, y, z)
#define NANI_TracySecureFreeS(x, y)
#define NANI_TracySecureMemoryDiscardS(x, y)

#define NANI_TracyAllocNS(x, y, z, w)
#define NANI_TracyFreeNS(x, y, z)
#define NANI_TracySecureAllocNS(x, y, z, w)
#define NANI_TracySecureFreeNS(x, y, z)

#define NANI_TracyMessageS(x, y, z)
#define NANI_TracyMessageLS(x, y)
#define NANI_TracyMessageCS(x, y, z, w)
#define NANI_TracyMessageLCS(x, y, z)

#define NANI_TracySourceCallbackRegister(x, y)
#define NANI_TracyParameterRegister(x, y)
#define NANI_TracyParameterSetup(x, y, z, w)
#define NANI_TracyIsConnected false
#define NANI_TracyIsStarted false
#define NANI_TracySetProgramName(x)

#define NANI_TracyFiberEnter(x)
#define NANI_TracyFiberEnterHint(x, y)
#define NANI_TracyFiberLeave

#endif
