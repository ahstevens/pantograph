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

class StudyManager
{
public:
	static StudyManager* getInstance();
	
	void init(std::string participant, bool isRightHanded, unsigned int nConditions, unsigned int nBlocks, unsigned int nRepsPerBlock);
	void next();
	void end();

	void logData(std::string type = "default", glm::vec3 *cursorPos = nullptr, Filament *filament = nullptr);

private:
	StudyManager();
	~StudyManager();

	static StudyManager *instance;
	
	bool fileExists(const std::string &fname);

	void prepareOutput(std::string name);

	bool snapshotTGA(std::string filename, bool append_timestamp = true);

	std::string intToString(int i, unsigned int pad_to_magnitude = 0);


	enum MODE { MOUSE = 0, PANTO = 1 };


	struct Record {
		std::string *participant;
	};

	std::ofstream outFile;
	std::string outFileName;

	std::string participant;
	bool rightHanded;
	unsigned int trial;
	unsigned int block;
	unsigned int replicate;
	MODE currentMode;

	unsigned int nConditions, nBlocks, nRepsPerBlock, nTrialsPerBlock;

	std::vector<Record> log;
};

