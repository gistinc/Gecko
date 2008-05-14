#ifndef __EmbeddingSetup_h_
#define __EmbeddingSetup_h_

#include "prtypes.h"
typedef PRUint32 nsresult;

/**
 * Initializes embedding, i.e. loads needed libs do other needed setup.
 *
 * You can call this as many times you like,
 * it will only do the initialization once.
 * 
 * Shutdown wil be handled automatically at exit.
 */
nsresult InitEmbedding();

#endif // __EmbeddingSetup_h_
