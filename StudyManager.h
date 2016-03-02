#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <sys/stat.h> // stat()
#include <ctime>

#include <glm\glm.hpp>
#include "Filament.h"
#include "Stopwatch.h"
#include "Cosmo.h"

class StudyManager
{
public:
	enum InteractionMode {
		ERR,
		NONE,
		MOUSE,
		PANTOGRAPH,
		POLHEMUS
	};

	enum InteractionState {
		STATE_ERR,
		ACTIVE,
		OFF
	};

	static StudyManager* getInstance();
	
	void init(Cosmo *cosmo, std::string participant, bool isRightHanded, unsigned int nConditions, unsigned int nBlocks, unsigned int nRepsPerBlock);
	void next();
	void end();

	void startTrial();

	bool isStudyStarted();
	bool isTrialStarted();

	void resetClock();

	void logData(std::string type = "default", glm::vec3 *cursorPos = nullptr, Filament *filament = nullptr, float *cursorDist = nullptr);

	bool snapshotTGA(std::string filename, bool append_timestamp = true);

	bool isSubjectLeftHanded();

	float getEyeSeparation();

	InteractionMode modeRestriction;
	InteractionMode currentMode;
	InteractionState currentState;

private:	
	struct Condition {
		InteractionMode interaction;
		float eye_separation;
		bool motion;
	};

	static StudyManager *instance;
	StudyManager();
	~StudyManager();
	
	bool fileExists(const std::string &fname);

	void prepareOutput(std::string name);

	std::string intToString(int i, unsigned int pad_to_magnitude = 0);

	Cosmo *cosmo;

	Stopwatch clock;

	std::vector< std::vector< std::vector< Condition > > > blocks;

	std::ofstream outFile;
	std::string outFileName;

	std::string participant;
	bool rightHanded;

	unsigned int nConditions, nBlocks, nRepsPerBlock, nTrialsPerBlock;

	float eyeSeparation;
	bool motion;

	bool studyStarted, trialStarted;
};

