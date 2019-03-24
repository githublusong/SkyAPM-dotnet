/*
 * Licensed to the SkyAPM under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The SkyAPM licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef CLR_PROFILER_CORPROFILER_H_
#define CLR_PROFILER_CORPROFILER_H_

#include <mutex>
#include <atomic>
#include <unordered_map>
#include "cor.h"
#include "corprof.h"
#include "clr_helpers.h"
#include "il_rewriter.h"
#include "config_loader.h"

namespace clrprofiler {

    class CorProfiler : public ICorProfilerCallback8
    {
    private:
        std::atomic<int> refCount;
        // this project agent support net461+ , if support net45 use ICorProfilerInfo4
        ICorProfilerInfo8* corProfilerInfo;
        std::mutex mapLock;

        //clrProfilerHomeEnvValue
        WSTRING clrProfilerHomeEnvValue;

        //iLRewriteMap ,because generic method has multi functionid
        std::unordered_map<mdMethodDef, bool> iLRewriteMap{};

        AssemblyProperty corAssemblyProperty{};
        bool entryPointReWrote = false;

        //moduleMetaInfoMap
        std::unordered_map<ModuleID, ModuleMetaInfo*> moduleMetaInfoMap{};

        //TraceConfig
        TraceConfig traceConfig;
    public:
        CorProfiler();
        virtual ~CorProfiler();
        HRESULT STDMETHODCALLTYPE Initialize(IUnknown* pICorProfilerInfoUnk) override;
        HRESULT STDMETHODCALLTYPE Shutdown() override;
        HRESULT STDMETHODCALLTYPE AppDomainCreationStarted(AppDomainID appDomainId) override;
        HRESULT STDMETHODCALLTYPE AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus) override;
        HRESULT STDMETHODCALLTYPE AppDomainShutdownStarted(AppDomainID appDomainId) override;
        HRESULT STDMETHODCALLTYPE AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus) override;
        HRESULT STDMETHODCALLTYPE AssemblyLoadStarted(AssemblyID assemblyId) override;
        HRESULT STDMETHODCALLTYPE AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus) override;
        HRESULT STDMETHODCALLTYPE AssemblyUnloadStarted(AssemblyID assemblyId) override;
        HRESULT STDMETHODCALLTYPE AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus) override;
        HRESULT STDMETHODCALLTYPE ModuleLoadStarted(ModuleID moduleId) override;
        HRESULT STDMETHODCALLTYPE ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus) override;
        HRESULT STDMETHODCALLTYPE ModuleUnloadStarted(ModuleID moduleId) override;
        HRESULT STDMETHODCALLTYPE ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus) override;
        HRESULT STDMETHODCALLTYPE ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId) override;
        HRESULT STDMETHODCALLTYPE ClassLoadStarted(ClassID classId) override;
        HRESULT STDMETHODCALLTYPE ClassLoadFinished(ClassID classId, HRESULT hrStatus) override;
        HRESULT STDMETHODCALLTYPE ClassUnloadStarted(ClassID classId) override;
        HRESULT STDMETHODCALLTYPE ClassUnloadFinished(ClassID classId, HRESULT hrStatus) override;
        HRESULT STDMETHODCALLTYPE FunctionUnloadStarted(FunctionID functionId) override;
        HRESULT STDMETHODCALLTYPE JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock) override;
        HRESULT STDMETHODCALLTYPE JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock) override;
        HRESULT STDMETHODCALLTYPE JITCachedFunctionSearchStarted(FunctionID functionId, BOOL* pbUseCachedFunction) override;
        HRESULT STDMETHODCALLTYPE JITCachedFunctionSearchFinished(FunctionID functionId, COR_PRF_JIT_CACHE result) override;
        HRESULT STDMETHODCALLTYPE JITFunctionPitched(FunctionID functionId) override;
        HRESULT STDMETHODCALLTYPE JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline) override;
        HRESULT STDMETHODCALLTYPE ThreadCreated(ThreadID threadId) override;
        HRESULT STDMETHODCALLTYPE ThreadDestroyed(ThreadID threadId) override;
        HRESULT STDMETHODCALLTYPE ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId) override;
        HRESULT STDMETHODCALLTYPE RemotingClientInvocationStarted() override;
        HRESULT STDMETHODCALLTYPE RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync) override;
        HRESULT STDMETHODCALLTYPE RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync) override;
        HRESULT STDMETHODCALLTYPE RemotingClientInvocationFinished() override;
        HRESULT STDMETHODCALLTYPE RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync) override;
        HRESULT STDMETHODCALLTYPE RemotingServerInvocationStarted() override;
        HRESULT STDMETHODCALLTYPE RemotingServerInvocationReturned() override;
        HRESULT STDMETHODCALLTYPE RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync) override;
        HRESULT STDMETHODCALLTYPE UnmanagedToManagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason) override;
        HRESULT STDMETHODCALLTYPE ManagedToUnmanagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason) override;
        HRESULT STDMETHODCALLTYPE RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason) override;
        HRESULT STDMETHODCALLTYPE RuntimeSuspendFinished() override;
        HRESULT STDMETHODCALLTYPE RuntimeSuspendAborted() override;
        HRESULT STDMETHODCALLTYPE RuntimeResumeStarted() override;
        HRESULT STDMETHODCALLTYPE RuntimeResumeFinished() override;
        HRESULT STDMETHODCALLTYPE RuntimeThreadSuspended(ThreadID threadId) override;
        HRESULT STDMETHODCALLTYPE RuntimeThreadResumed(ThreadID threadId) override;
        HRESULT STDMETHODCALLTYPE MovedReferences(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[]) override;
        HRESULT STDMETHODCALLTYPE ObjectAllocated(ObjectID objectId, ClassID classId) override;
        HRESULT STDMETHODCALLTYPE ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[]) override;
        HRESULT STDMETHODCALLTYPE ObjectReferences(ObjectID objectId, ClassID classId, ULONG cObjectRefs, ObjectID objectRefIds[]) override;
        HRESULT STDMETHODCALLTYPE RootReferences(ULONG cRootRefs, ObjectID rootRefIds[]) override;
        HRESULT STDMETHODCALLTYPE ExceptionThrown(ObjectID thrownObjectId) override;
        HRESULT STDMETHODCALLTYPE ExceptionSearchFunctionEnter(FunctionID functionId) override;
        HRESULT STDMETHODCALLTYPE ExceptionSearchFunctionLeave() override;
        HRESULT STDMETHODCALLTYPE ExceptionSearchFilterEnter(FunctionID functionId) override;
        HRESULT STDMETHODCALLTYPE ExceptionSearchFilterLeave() override;
        HRESULT STDMETHODCALLTYPE ExceptionSearchCatcherFound(FunctionID functionId) override;
        HRESULT STDMETHODCALLTYPE ExceptionOSHandlerEnter(UINT_PTR __unused) override;
        HRESULT STDMETHODCALLTYPE ExceptionOSHandlerLeave(UINT_PTR __unused) override;
        HRESULT STDMETHODCALLTYPE ExceptionUnwindFunctionEnter(FunctionID functionId) override;
        HRESULT STDMETHODCALLTYPE ExceptionUnwindFunctionLeave() override;
        HRESULT STDMETHODCALLTYPE ExceptionUnwindFinallyEnter(FunctionID functionId) override;
        HRESULT STDMETHODCALLTYPE ExceptionUnwindFinallyLeave() override;
        HRESULT STDMETHODCALLTYPE ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId) override;
        HRESULT STDMETHODCALLTYPE ExceptionCatcherLeave() override;
        HRESULT STDMETHODCALLTYPE COMClassicVTableCreated(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable, ULONG cSlots) override;
        HRESULT STDMETHODCALLTYPE COMClassicVTableDestroyed(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable) override;
        HRESULT STDMETHODCALLTYPE ExceptionCLRCatcherFound() override;
        HRESULT STDMETHODCALLTYPE ExceptionCLRCatcherExecute() override;
        HRESULT STDMETHODCALLTYPE ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[]) override;
        HRESULT STDMETHODCALLTYPE GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason) override;
        HRESULT STDMETHODCALLTYPE SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[]) override;
        HRESULT STDMETHODCALLTYPE GarbageCollectionFinished() override;
        HRESULT STDMETHODCALLTYPE FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID) override;
        HRESULT STDMETHODCALLTYPE RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[]) override;
        HRESULT STDMETHODCALLTYPE HandleCreated(GCHandleID handleId, ObjectID initialObjectId) override;
        HRESULT STDMETHODCALLTYPE HandleDestroyed(GCHandleID handleId) override;
        HRESULT STDMETHODCALLTYPE InitializeForAttach(IUnknown* pCorProfilerInfoUnk, void* pvClientData, UINT cbClientData) override;
        HRESULT STDMETHODCALLTYPE ProfilerAttachComplete() override;
        HRESULT STDMETHODCALLTYPE ProfilerDetachSucceeded() override;
        HRESULT STDMETHODCALLTYPE ReJITCompilationStarted(FunctionID functionId, ReJITID rejitId, BOOL fIsSafeToBlock) override;
        HRESULT STDMETHODCALLTYPE GetReJITParameters(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl* pFunctionControl) override;
        HRESULT STDMETHODCALLTYPE ReJITCompilationFinished(FunctionID functionId, ReJITID rejitId, HRESULT hrStatus, BOOL fIsSafeToBlock) override;
        HRESULT STDMETHODCALLTYPE ReJITError(ModuleID moduleId, mdMethodDef methodId, FunctionID functionId, HRESULT hrStatus) override;
        HRESULT STDMETHODCALLTYPE MovedReferences2(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], SIZE_T cObjectIDRangeLength[]) override;
        HRESULT STDMETHODCALLTYPE SurvivingReferences2(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], SIZE_T cObjectIDRangeLength[]) override;
        HRESULT STDMETHODCALLTYPE ConditionalWeakTableElementReferences(ULONG cRootRefs, ObjectID keyRefIds[], ObjectID valueRefIds[], GCHandleID rootIds[]) override;
        HRESULT STDMETHODCALLTYPE GetAssemblyReferences(const WCHAR* wszAssemblyPath, ICorProfilerAssemblyReferenceProvider* pAsmRefProvider) override;
        HRESULT STDMETHODCALLTYPE ModuleInMemorySymbolsUpdated(ModuleID moduleId) override;

        HRESULT STDMETHODCALLTYPE DynamicMethodJITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock, LPCBYTE ilHeader, ULONG cbILHeader) override;
        HRESULT STDMETHODCALLTYPE DynamicMethodJITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock) override;

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override
        {
            if (riid == __uuidof(ICorProfilerCallback8) ||
                riid == __uuidof(ICorProfilerCallback7) ||
                riid == __uuidof(ICorProfilerCallback6) ||
                riid == __uuidof(ICorProfilerCallback5) ||
                riid == __uuidof(ICorProfilerCallback4) ||
                riid == __uuidof(ICorProfilerCallback3) ||
                riid == __uuidof(ICorProfilerCallback2) ||
                riid == __uuidof(ICorProfilerCallback) ||
                riid == IID_IUnknown)
            {
                *ppvObject = this;
                this->AddRef();
                return S_OK;
            }

            *ppvObject = nullptr;
            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef(void) override
        {
            return std::atomic_fetch_add(&this->refCount, 1) + 1;
        }

        ULONG STDMETHODCALLTYPE Release(void) override
        {
            int count = std::atomic_fetch_sub(&this->refCount, 1) - 1;

            if (count <= 0)
            {
                delete this;
            }

            return count;
        }
    };
}

#endif
