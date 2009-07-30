/*
 *  Db.cpp
 *  CreateBerkeleyPt
 *
 *  Created by Hieu Hoang on 29/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include "DbWrapper.h"
#include "Vocab.h"
#include "Phrase.h"
#include "Global.h"

using namespace std;

DbWrapper::DbWrapper()
:m_dbMisc(0, 0)
,m_dbVocab(0, 0)
,m_dbSource(0, 0)
,m_dbTarget(0, 0)
,m_nextSourceId(1)
{}

DbWrapper::~DbWrapper()
{
	m_dbVocab.close(0);
}

void DbWrapper::Open(const string &filePath)
{
	m_dbMisc.set_error_stream(&cerr);
	m_dbMisc.set_errpfx("SequenceExample");
	m_dbMisc.open(NULL, (filePath + "/Misc.db").c_str(), NULL, DB_BTREE, DB_CREATE, 0664);

	m_dbVocab.set_error_stream(&cerr);
	m_dbVocab.set_errpfx("SequenceExample");
	m_dbVocab.open(NULL, (filePath + "/Vocab.db").c_str(), NULL, DB_BTREE, DB_CREATE, 0664);

	m_dbSource.set_error_stream(&cerr);
	m_dbSource.set_errpfx("SequenceExample");
	m_dbSource.open(NULL, (filePath + "/Source.db").c_str(), NULL, DB_BTREE, DB_CREATE, 0664);

	m_dbTarget.set_error_stream(&cerr);
	m_dbTarget.set_errpfx("SequenceExample");
	m_dbTarget.open(NULL, (filePath + "/Target.db").c_str(), NULL, DB_BTREE, DB_CREATE, 0664);
	
}

void DbWrapper::Save(const Vocab &vocab)
{
	Vocab::const_iterator iterVocab;
	for (iterVocab = vocab.begin(); iterVocab != vocab.end(); ++iterVocab)
	{
		const string &word = iterVocab->first;
		char *wordChar = (char*) malloc(word.size() + 1);
		strcpy(wordChar, word.c_str());
		VocabId vocabId = iterVocab->second;
		
		cerr << word << " = " << vocabId << endl;
		
		Dbt key(wordChar, word.size() + 1);
		Dbt data(&vocabId, sizeof(VocabId));
		
		int ret = m_dbVocab.put(NULL, &key, &data, DB_NOOVERWRITE);
		if (ret == DB_KEYEXIST) 
		{
			m_dbVocab.err(ret, "Put failed because key %f already exists", wordChar);
		}
		
		free(wordChar);
	}
	
}

void DbWrapper::GetAllVocab()
{
	Dbt key, data;
	
	// search
	char *c = "Less";
	key.set_data(c);
	key.set_size(5);
	
	m_dbVocab.get(NULL, &key, &data, 0);
	
	VocabId *id = (VocabId*) data.get_data();
	
	cerr << *id << endl;
	
	// cursors
	Dbc *cursorp;
	m_dbVocab.cursor(NULL, &cursorp, 0); 
	
	int ret;
	
	// Iterate over the database, retrieving each record in turn.
	while ((ret = cursorp->get(&key, &data, DB_NEXT)) == 0)
	{
		VocabId *id = (VocabId*) data.get_data();
		
		cerr << key.get_size() << " "
		<< (char*) key.get_data() << " = "
		<< data.get_size() << " "
		<< *id << endl;
		
	}
	if (ret != DB_NOTFOUND) {
		// ret should be DB_NOTFOUND upon exiting the loop.
		// Dbc::get() will by default throw an exception if any
		// significant errors occur, so by default this if block
		// can never be reached. 
		cerr << "baar";
	}
		
}

void DbWrapper::SaveSource(const Phrase &source, const Phrase &target)
{
	long currSourceId = 0;
	
	// SOURCE
	for (size_t pos = 0; pos < source.GetSize(); ++pos)
	{
		const Word &word = source.GetWord(pos);
		currSourceId = SaveSourceWord(currSourceId, word);
									 		
		if (word.IsNonTerminal())
		{ // store the TARGET non-term label straight after source non-term label
			size_t targetPos = target.GetAlign(pos);
			const Word &targetWord = target.GetWord(targetPos);
			currSourceId = SaveSourceWord(currSourceId, targetWord);
		}
	}
}

long DbWrapper::SaveSourceWord(long currSourceId, const Word &word)
{
	long retSourceId;
	
	// create db data
	SourceKey sourceKey(currSourceId, word.GetFactor(0));
	long nextSourceId = m_nextSourceId;
	
	Dbt key(&sourceKey, sizeof(SourceKey));
	Dbt data(&nextSourceId, sizeof(long));
	
	// save
	int ret = m_dbSource.put(NULL, &key, &data, DB_NOOVERWRITE);
	if (ret == DB_KEYEXIST) 
	{ // already exist. get node id
		m_dbSource.get(NULL, &key, &data, 0);
		
		long *sourceId = (long*) data.get_data();
		retSourceId = *sourceId;
	}
	else
	{
		retSourceId = m_nextSourceId;
		++m_nextSourceId;
	}
	
	return retSourceId;
}

void DbWrapper::SaveTarget(const Phrase &phrase)
{
	
	// allocate mem
	const Global &global = Global::Instance();

	size_t memNeeded = global.GetSourceWordSize() + global.GetTargetWordSize();
	memNeeded += sizeof(int) +  global.GetTargetWordSize() * phrase.GetSize(); // phrase
	memNeeded += sizeof(int) + 2 * sizeof(int) * phrase.GetAlign().size(); // align
	memNeeded += sizeof(float) * global.GetNumScores(); // scores
	
	char *mem = (char*) malloc(memNeeded);

	size_t memUsed = 0;

	// head words
	memUsed += phrase.GetHeadWords(0).WriteToMemory(mem);
	memUsed += phrase.GetHeadWords(1).WriteToMemory(mem + memUsed);
	
	// phrase
	/// size
	int phraseSize = phrase.GetSize();
	memcpy(mem + memUsed, &phraseSize, sizeof(int));
	memUsed += sizeof(int);

	// word
	for (size_t pos = 0; pos < phrase.GetSize(); ++pos)
	{
		const Word &word = phrase.GetWord(pos);
		memUsed += word.WriteToMemory(mem + memUsed);
	}
	
	// align
	memUsed += phrase.WriteAlignToMemory(mem + memUsed);
	
	// scores
	memUsed += phrase.WriteScoresToMemory(mem + memUsed);

	assert(memNeeded == memUsed);
	
}


