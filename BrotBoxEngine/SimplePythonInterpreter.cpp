#include "BBE/SimplePythonInterpreter.h"
#include "BBE/SimpleFile.h"
#include "BBE/Logging.h"
#include "BBE/Async.h"
#include <Python.h>

void bbe::simplePython::interpret(const bbe::String& code)
{
	static bool initRequired = true;
	if (initRequired)
	{
		initRequired = false;
		Py_Initialize();
	}
	auto state = PyGILState_Ensure();
	BBELOGLN(code);
	PyRun_SimpleString(code.getRaw());
	PyGILState_Release(state);
}

bbe::Sound bbe::simplePython::gtts(const bbe::String& text, const bbe::String& lang)
{
	const bbe::String temp = "temporaryGTTSFile.mp3";
	bbe::simpleFile::deleteFile(temp);

	bbe::String code = "from gtts import gTTS\n";
	code += "text = '" + text.replace("'", "\'") + "'\n";
	code += "tts = gTTS(text=text, lang='" + lang + "')\n";
	code += "tts.save('" + temp + "')\n";
	
	interpret(code);

	if (!simpleFile::doesFileExist(temp))
	{
		bbe::Crash(bbe::Error::IllegalState, "Python interpreter did not create GTTS file!");
	}

	bbe::Sound retVal(temp);
	
	bbe::simpleFile::deleteFile(temp);

	return retVal;
}

std::future<bbe::Sound> bbe::simplePython::gttsAsync(const bbe::String& text, const bbe::String& lang)
{
	return bbe::async(&gtts, text, lang);
}
