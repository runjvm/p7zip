/* MatchFinderMt.h */

#ifndef __MATCHFINDERMT_H
#define __MATCHFINDERMT_H

#include "../../Threads.h"
#include "MatchFinder.h"

#define kMtHashBlockSize (1 << 13)
#define kMtHashNumBlocks (1 << 3)
#define kMtHashNumBlocksMask (kMtHashNumBlocks - 1)

#define kMtBtBlockSize (1 << 14)
#define kMtBtNumBlocks (1 << 6)
#define kMtBtNumBlocksMask (kMtBtNumBlocks - 1)

typedef struct _CMtSync
{
  Bool wasCreated;
  Bool needStart;
  Bool exit;
  Bool stopWriting;

  CThread thread;
  CAutoResetEvent canStart;
  CAutoResetEvent wasStarted;
  CAutoResetEvent wasStopped;
  CSemaphore freeSemaphore;
  CSemaphore filledSemaphore;
  Bool csWasInitialized;
  Bool csWasEntered;
  CCriticalSection cs;
  UInt32 numProcessedBlocks;
} CMtSync;

typedef UInt32 * (*Mf_Mix_Matches)(void *p, UInt32 matchMinPos, UInt32 *distances);

/* kMtCacheLineDummy must be >= size_of_CPU_cache_line */
#define kMtCacheLineDummy 128

typedef void (*Mf_GetHeads)(const Byte *buffer, size_t pos,
  UInt32 *hash, UInt32 hashMask, UInt32 *heads, UInt32 numHeads);

typedef struct _CMatchFinderMt
{
  /* LZ */
  const Byte *pointerToCurPos;
  UInt32 *btBuf;
  UInt32 btBufPos;
  UInt32 btBufPosLimit;
  UInt32 lzPos;
  UInt32 btNumAvailBytes;

  UInt32 *hash;
  UInt32 fixedHashSize;
  UInt32 historySize;

  Mf_Mix_Matches MixMatchesFunc;
  
  /* LZ + BT */
  CMtSync btSync;
  Byte btDummy[kMtCacheLineDummy];

  /* BT */
  UInt32 *hashBuf;
  UInt32 hashBufPos;
  UInt32 hashBufPosLimit;
  UInt32 hashNumAvail;

  CLzRef *son;
  UInt32 matchMaxLen;
  UInt32 numHashBytes;
  UInt32 pos;
  Byte *buffer;   // Pointer to virtual Buffer begin
  UInt32 cyclicBufferPos;
  UInt32 cyclicBufferSize; // it must be historySize + 1
  UInt32 cutValue;

  /* BT + Hash */
  CMtSync hashSync;
  /* Byte hashDummy[kMtCacheLineDummy]; */
  
  /* Hash */
  Mf_GetHeads GetHeadsFunc;
  CMatchFinder *MatchFinder;
} CMatchFinderMt;

void MatchFinderMt_Construct(CMatchFinderMt *p);
void MatchFinderMt_Destruct(CMatchFinderMt *p, ISzAlloc *alloc);
HRes MatchFinderMt_Create(CMatchFinderMt *p, UInt32 historySize, UInt32 keepAddBufferBefore, 
    UInt32 matchMaxLen, UInt32 keepAddBufferAfter, ISzAlloc *alloc);
void MatchFinderMt_CreateVTable(CMatchFinderMt *p, IMatchFinder *vTable);
void MatchFinderMt_ReleaseStream(CMatchFinderMt *p);

#endif
