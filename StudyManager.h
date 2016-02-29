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

class StudyManager
{
public:
	enum InteractionMode {
		ERR,
		TRAINING,
		MOUSE,
		PANTOGRAPH,
		POLHEMUS
	};

	static StudyManager* getInstance();
	
	void init(std::string participant, bool isRightHanded, unsigned int nConditions, unsigned int nBlocks, unsigned int nRepsPerBlock);
	void next();
	void end();

	void resetClock();

	void logData(std::string type = "default", glm::vec3 *cursorPos = nullptr, Filament *filament = nullptr, float *cursorDist = nullptr);

	bool snapshotTGA(std::string filename, bool append_timestamp = true);

	bool isSubjectLeftHanded();

private:	
	static StudyManager *instance;
	StudyManager();
	~StudyManager();
	
	bool fileExists(const std::string &fname);

	void prepareOutput(std::string name);

	std::string intToString(int i, unsigned int pad_to_magnitude = 0);

	Stopwatch clock;

	std::ofstream outFile;
	std::string outFileName;

	std::string participant;
	bool rightHanded;
	unsigned int trial;
	unsigned int block;
	unsigned int replicate;
	InteractionMode currentMode;

	unsigned int nConditions, nBlocks, nRepsPerBlock, nTrialsPerBlock;
};

