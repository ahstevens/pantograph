#include "StudyManager.h"

// Initialize class variables
StudyManager* StudyManager::instance = NULL;

// Returns singleton StudyManager instance
StudyManager* StudyManager::getInstance()
{
	if (!instance)
		instance = new StudyManager();
	return instance;
}

StudyManager::StudyManager()
{
}


StudyManager::~StudyManager()
{
}



void StudyManager::init(Cosmo *cosmo, std::string participant, bool isRightHanded, unsigned int nBlocks, unsigned int nRepsPerBlock)
{
	motion = stereo = true;

	studyStarted = trialStarted = done = false;

	modeRestriction = currentMode = NONE;
	currentState = OFF;

	this->cosmo = cosmo;
	this->participant = participant;
	this->rightHanded = isRightHanded;
	this->nBlocks = nBlocks;
	this->nRepsPerBlock = nRepsPerBlock;


	// GENERATE TRIAL BLOCKS
	std::vector< InteractionMode > interactions;
	interactions.push_back( MOUSE );
	interactions.push_back( PANTOGRAPH );
	interactions.push_back( POLHEMUS );

	std::vector< bool > stereoscopy;
	stereoscopy.push_back( true );
	stereoscopy.push_back( false );

	std::vector< bool > oscillation;
	oscillation.push_back( true );
	oscillation.push_back( false );


	std::vector< std::vector<Condition> > block;

	for (int i = 0; i < interactions.size(); ++i)
		for (int j = 0; j < stereoscopy.size(); ++j)
			for (int k = 0; k < oscillation.size(); ++k)
			{
				std::vector<Condition> replicates;

				Condition c;
				c.interaction = interactions[ i ];
				c.stereo = stereoscopy[ j ];
				c.motion = oscillation[ k ];

				for (int l = 0; l < nRepsPerBlock; ++l)
					replicates.push_back( c );

				block.push_back(replicates);			
			}


	for (int i = 0; i < nBlocks; ++i)
	{
		std::random_shuffle(block.begin(), block.end());

		blocks.push_back(block);
	}

	this->nConditions = interactions.size() * stereoscopy.size() * oscillation.size();
	
	this->nTrialsPerBlock = nRepsPerBlock * this->nConditions;

	prepareOutput(participant);
}

void StudyManager::begin()
{
	studyStarted = true;

	cosmo->hideOscillationAxis();
	cosmo->setLensType(Cosmo::Lens::SPHERE_POINTS);

	std::cout << "Commencing study..." << std::endl;
	std::cout << std::endl;

	next();
}

void StudyManager::next()
{
	if (studyStarted)
	{
		// get current trial block from queue
		std::vector< std::vector< Condition > > *block = &blocks.back();
		std::vector< Condition > *cond = &block->back();

		if (cond->size() == 0)
		{
			std::cout << "Block " << nBlocks - blocks.size() + 1 << ": Condition " << nConditions - block->size() + 1 << " of " << nConditions << " completed!" << std::endl;
			block->pop_back();

			if (block->size() == 0) // BLOCK DONE
			{
				std::cout << "Block " << nBlocks - blocks.size() + 1 << " of " << nBlocks << " completed!" << std::endl;
				blocks.pop_back();

				if (blocks.size() == 0)
				{
					end();
					return;
				}
				else
					block = &blocks.back();

				//std::cout << "Please take a short break, then press the ENTER key when ready to begin the next block." << std::endl << std::endl;
			}
			else // CONDITION REPLICATES DONE
			{
				//std::cout << "Please take a short break, then press the ENTER key when ready to begin the next condition." << std::endl << std::endl;
			}

			cond = &block->back();
		}


		Condition *repl = &cond->back();
		cond->pop_back();

		modeRestriction = repl->interaction;
		stereo = repl->stereo;
		motion = repl->motion;

		clock.start();
		trialStarted = false;
	}

	cosmo->generateFilament();
	cosmo->setMovableRotationCenter(glm::vec3(0.f));
}

void StudyManager::end()
{
	std::cout << std::endl;
	std::cout << "============================================================" << std::endl;
	std::cout << "| >>>>>>>>>>>>>>>>>>>> STUDY COMPLETE <<<<<<<<<<<<<<<<<<<< |" << std::endl;
	std::cout << "============================================================" << std::endl;
	std::cout << std::endl;
	done = true;
}

void StudyManager::startTrial() { trialStarted = true; }

bool StudyManager::isStudyStarted() { return studyStarted; }

bool StudyManager::isTrialStarted() { return trialStarted; }

bool StudyManager::isStudyDone() { return done; }

bool StudyManager::isSubjectLeftHanded() { return !rightHanded; }

void StudyManager::toggleStereo() { stereo = !stereo; }

bool StudyManager::getStereo() { return stereo; }

void StudyManager::toggleMotion() { motion = !motion; }

bool StudyManager::getMotion() { return motion; }

float StudyManager::getProgress() { 
	float trialsRemaining = (float)((blocks.size() - 1) * nTrialsPerBlock + (blocks.back().size() - 1) * nRepsPerBlock + blocks.back().back().size() + 1);
	float nTrials = (float)(nTrialsPerBlock * nBlocks);
	float trialsCompleted = nTrials - trialsRemaining;
	return (trialsCompleted / nTrials) * 100.f; 
}

bool StudyManager::fileExists(const std::string &fname)
{
	struct stat buffer;
	return (stat(fname.c_str(), &buffer) == 0);
}

void StudyManager::prepareOutput(std::string name)
{
	// construct filename
	outFileName = std::string("data/" + name + "_data" + ".csv");

	// if file exists, keep trying until we find a filename that doesn't already exist
	for (int i = 0; fileExists(outFileName); ++i)
		outFileName = std::string("data/" + name + "_data_" + std::to_string(i) + ".csv");

	outFile.open(outFileName);

	if (outFile.is_open())
	{
		std::cout << "Opened file " << outFileName << " for writing output" << std::endl;
		outFile << "participant,handedness,";
		outFile << "block,trial,";
		outFile << "interaction,stereo,motion,";
		outFile << "log_type,";
		outFile << "cursor.x,cursor.y,cursor.z,";
		outFile << "cursorDist,";
		outFile << "filament.cp0.x,filament.cp0.y,filament.cp0.z,";
		outFile << "filament.cp1.x,filament.cp1.y,filament.cp1.z,";
		outFile << "filament.cp2.x,filament.cp2.y,filament.cp2.z,";
		outFile << "filament.cp3.x,filament.cp3.y,filament.cp3.z,";
		outFile << "filament.length,filament.radius,";
		outFile << "cursor_to_filament_min_dist,";
		outFile << "time" << std::endl;
	}
	else
		std::cout << "Error opening file " << outFileName << " for writing output" << std::endl;
}

void StudyManager::logData(std::string type, glm::vec3 *cursorPos, Filament *filament, float *cursorDist, bool resetClock)
{
	double time = clock.read();

	if (resetClock) clock.start();


	// Construct string for rendering mode enum
	std::string conditionString;

	assert(currentMode != ERR);

	switch (currentMode)
	{
	case MOUSE:
		conditionString = std::string("mouse");
		break;
	case PANTOGRAPH:
		conditionString = std::string("panto");
		break;
	case POLHEMUS:
		conditionString = std::string("polhemus");
		break;
	default:
		conditionString = std::string("none");
	}

	// Begin outputting trial into file
	outFile << participant << ",";
	outFile << (rightHanded ? "R" : "L") << ",";
	outFile << ( !studyStarted ? 0 : ( nBlocks - blocks.size() ) ) << ",";
	outFile << ( !studyStarted ? 0 : ( nTrialsPerBlock - ( ( blocks.back().size() - 1 ) * nRepsPerBlock + blocks.back().back().size() ) ) ) << ",";
	outFile << conditionString << ",";
	outFile << stereo << ",";
	outFile << motion << ",";
	outFile << type << ",";
	outFile << (cursorPos ? std::to_string(cursorPos->x) : "") << ",";
	outFile << (cursorPos ? std::to_string(cursorPos->y) : "") << ",";
	outFile << (cursorPos ? std::to_string(cursorPos->z) : "") << ",";
	outFile << (cursorDist ? std::to_string(*cursorDist) : "") << ",";
	outFile << (filament ? std::to_string(filament->getSplineControlPoint(0).x) : "") << ",";
	outFile << (filament ? std::to_string(filament->getSplineControlPoint(0).y) : "") << ",";
	outFile << (filament ? std::to_string(filament->getSplineControlPoint(0).z) : "") << ",";
	outFile << (filament ? std::to_string(filament->getSplineControlPoint(1).x) : "") << ",";
	outFile << (filament ? std::to_string(filament->getSplineControlPoint(1).y) : "") << ",";
	outFile << (filament ? std::to_string(filament->getSplineControlPoint(1).z) : "") << ",";
	outFile << (filament ? std::to_string(filament->getSplineControlPoint(2).x) : "") << ",";
	outFile << (filament ? std::to_string(filament->getSplineControlPoint(2).y) : "") << ",";
	outFile << (filament ? std::to_string(filament->getSplineControlPoint(2).z) : "") << ",";
	outFile << (filament ? std::to_string(filament->getSplineControlPoint(3).x) : "") << ",";
	outFile << (filament ? std::to_string(filament->getSplineControlPoint(3).y) : "") << ",";
	outFile << (filament ? std::to_string(filament->getSplineControlPoint(3).z) : "") << ",";
	outFile << (filament ? std::to_string(filament->getLength()) : "") << ",";
	outFile << (filament ? std::to_string(filament->getRadius()) : "") << ",";
	outFile << (filament && cursorPos ? std::to_string(filament->getMinDistTo(cursorPos)) : "") << ",";
	outFile << time << std::endl;

}

bool StudyManager::snapshotTGA(std::string filename, bool append_timestamp)
{
	// get frame buffer size
	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);

	//This prevents the images getting padded 
	// when the width multiplied by 3 is not a multiple of 4
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	int nSize = w*h * 3;
	// First let's create our buffer, 3 channels per Pixel
	char* dataBuffer = (char*)malloc(nSize*sizeof(char));

	if (!dataBuffer) return false;

	// Let's fetch them from the backbuffer	
	// We request the pixels in GL_BGR format, thanks to Berzeger for the tip
	glReadPixels((GLint)0, (GLint)0,
		(GLint)w, (GLint)h,
		GL_BGR, GL_UNSIGNED_BYTE, dataBuffer);

	if (append_timestamp)
	{
		time_t t = time(0);   // get time now
		struct tm *now = localtime(&t);

		/*** DATE ***/
		// year
		filename += "_" + intToString(now->tm_year + 1900, 3) + "-";

		// month
		filename += intToString(now->tm_mon + 1, 1) + "-";

		// day
		filename += intToString(now->tm_mday, 1);

		/*** TIME ***/
		// hour
		filename += "_" + intToString(now->tm_hour, 1);

		// minute
		filename += "-" + intToString(now->tm_min, 1);

		// second
		filename += "-" + intToString(now->tm_sec, 1);
	}

	filename = "snapshots\\" + filename + ".tga";

	//Now the file creation
	FILE *filePtr = fopen(std::string(filename).c_str(), "wb");
	if (!filePtr)
	{
		std::cerr << "ERROR: Could not save snapshot to " << filename << std::endl;
		std::cerr << "Make sure the path is valid and that the 'snapshots' directory exists and try again." << std::endl;
		return false;
	}


	unsigned char TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char header[6] = { w % 256, w / 256,
		h % 256, h / 256,
		24, 0 };
	// We write the headers
	fwrite(TGAheader, sizeof(unsigned char), 12, filePtr);
	fwrite(header, sizeof(unsigned char), 6, filePtr);
	// And finally our image data
	fwrite(dataBuffer, sizeof(GLubyte), nSize, filePtr);
	fclose(filePtr);

	std::cout << "Snapshot saved to " << filename << std::endl;

	return true;
}

std::string StudyManager::intToString(int i, unsigned int pad_to_magnitude)
{
	if (pad_to_magnitude < 1)
		return std::to_string(i);

	std::string ret;

	int mag = i == 0 ? 0 : (int)log10(i);

	for (int j = pad_to_magnitude - mag; j > 0; --j)
		ret += std::to_string(0);

	ret += std::to_string(i);

	return ret;
}