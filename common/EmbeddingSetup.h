#ifndef __EmbeddingSetup_h_
#define __EmbeddingSetup_h_

#include "prtypes.h"
typedef PRUint32 nsresult;

/**
 * Initializes embedding, i.e. loads needed libs do other needed setup.
 *
 * You can call this as many times you like,
 * it will only do the initialization once,
 * but be sure to call TermEmbedding a matching number of times.
 *
 * @param aProfilePath Optional argument to set the path where
 *  profile data is stored. The directory will be created if it
 *  doesn't exist.
 */
nsresult InitEmbedding(const char* aProfilePath = 0);

/**
 * Terminates embedding, i.e. does teardown and unloads needed libs.
 *
 * Make sure to match every call of InitEmbedding with a call
 * to TermEmbedding. Only the last call to TermEmbedding will do
 * the actual termination.
 */
nsresult TermEmbedding();

#endif // __EmbeddingSetup_h_
