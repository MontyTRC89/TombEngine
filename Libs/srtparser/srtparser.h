/*
 * Author   : Saurabh Shrivastava
 * Email    : saurabh.shrivastava54@gmail.com
 * Link     : https://github.com/saurabhshri
 *
 * Based on subtitle-parser by Oleksii Maryshchenko.
 * Email    : young_developer@mail.ru
 * Link     : https://github.com/young-developer/subtitle-parser
 */

#ifndef SRTPARSER_H
#define SRTPARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>

//function for splitting sentences based on supplied delimiter
inline std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

/**** Class definitions ****/


class SubtitleWord
{
private:
    std::string _text;
public:
    SubtitleWord(void);
    SubtitleWord(std::string text);
    virtual std::string getText() const;
    ~SubtitleWord(void);
};

class SubtitleItem
{
private:
    long int _startTime;                    //in milliseconds
    long int _endTime;
    std::string _text;                      //actual line, as present in subtitle file
    long int timeMSec(std::string value);   //converts time string into ms

    int _subNo;                              //subtitle number
    std::string _startTimeString;           //time as in srt format
    std::string _endTimeString;
    bool _ignore;                           //should subtitle be ignore; used when the subtitle is empty after processing
    std::string _justDialogue;              //contains processed subtitle - stripped style, non dialogue text removal etc.
    int _speakerCount;                      //count of number of speakers
    std::vector<std::string> _speaker;      //list of speakers in a single subtitle
    int _nonDialogueCount;                  //count of non spoken words in a subtitle
    std::vector<std::string> _nonDialogue;  //list of non dialogue words, e.g. (applause)
    int _wordCount;                         //number of words in _justDialogue
    std::vector<std::string> _word;         //list of words in dialogue
    std::vector<long int> _wordStartTime;   //start time of each word in dialogue
    std::vector<long int> _wordEndTime;     //end time of each word in dialogue
    std::vector<long int> _wordDuration;   //actual duration of each word without silence
    int _styleTagCount;                     //count of style tags in a single subtitle
    std::vector<std::string> _styleTag;     //list of style tags in that subtitle
    void extractInfo(bool keepHTML = 0, bool doNotIgnoreNonDialogues = 0,  bool doNotRemoveSpeakerNames = 0);   //process subtitle
public:
    long int getStartTime() const;          //returns starting time in ms
    long int getEndTime() const;            //returns ending time in ms
    std::string getText() const;            //returns subtitle text as present in .srt file

    int getSubNo() const;              //returns subtitle number
    std::string getStartTimeString() const; //returns sarting time as present in .srt file
    std::string getEndTimeString() const;   //returns ending time as present in .srt file
    bool getIgnoreStatus() const;           //returns status, whether the subtitle is ignorable or not after processing
    std::string getDialogue(bool keepHTML = 0, bool doNotIgnoreNonDialogues = 0,  bool doNotRemoveSpeakerNames = 0); //returns processed subtitle
    int getSpeakerCount() const;            //return speaker count
    int getNonDialogueCount() const;        //return non dialogue words count
    int getStyleTagCount() const;           //return style tags count
    int getWordCount() const;               //return words count
    std::vector<std::string> getIndividualWords(); //return string vector of individual words
    std::string getWordByIndex(int index);       //return word stored at 'index'
    std::vector<long int> getWordStartTimes();   //return long int vector of start time of individual words
    std::vector<long int> getWordEndTimes();     //return long int vector of end time of individual words
    long int getWordStartTimeByIndex(int index); //return the start time of a word based on index
    long int getWordEndTimeByIndex (int index);  //return the end time of a word based on index
    std::vector<std::string> getSpeakerNames();  //return string vector of speaker names
    std::vector<std::string> getNonDialogueWords(); //return string vector of non dialogue words
    std::vector<std::string> getStyleTags();    //return string vector of style tags


    void setStartTime(long int startTime);  //set starting time
    void setEndTime(long int endTime);      //set ending time
    void setText(std::string text);         //set subtitle text
    void setWordTimes(std::vector<long int> wordStartTime, std::vector<long int> wordEndTime, std::vector<long int> wordDuration);  //assign time to individual words

    SubtitleItem(void);
    SubtitleItem(int subNo, std::string startTime,std::string endTime, std::string text, bool ignore = false,
                 std::string justDialogue = "" , int speakerCount = 0, int nonDialogueCount = 0,
                 int styleTagCount = 0, int wordCount = 0, std::vector<std::string> speaker = std::vector<std::string>(),
                 std::vector<std::string> nonDialogue = std::vector<std::string>(),
                 std::vector<std::string> styleTags = std::vector<std::string>(),
                 std::vector<std::string> word = std::vector<std::string>());  //default constructor
    ~SubtitleItem(void);
};

class SubtitleParser
{
protected:
    std::vector<SubtitleItem*> _subtitles;              //stores subtitles
    std::string _fileName;                              //supplied filename
    virtual void parse(std::string fileName) = 0;
public:
    virtual std::vector<SubtitleItem*> getSubtitles();  //returns subtitles
    std::string getFileData();
    SubtitleParser(void);
    virtual ~SubtitleParser(void);
};

class SubtitleParserFactory
{
private:
    std::string _fileName;
public:
    SubtitleParser* getParser();
    SubtitleParserFactory(std::string fileName);
    ~SubtitleParserFactory(void);
};

class SubRipParser : public SubtitleParser
{
    void parse(std::string fileName);
public:
    SubRipParser(void);
    SubRipParser(std::string fileName);
    ~SubRipParser(void);
};


/**** Function definitions ****/

//1. SubtitleParserFactory class

inline SubtitleParserFactory::SubtitleParserFactory(std::string fileName)
{
    _fileName = fileName;
}

inline SubtitleParser* SubtitleParserFactory::getParser()
{
    return new SubRipParser(_fileName);                 //creates and returns SubRipParser obj
}

inline SubtitleParserFactory::~SubtitleParserFactory(void)
{
}

//2. SubtitleParser class

inline std::vector<SubtitleItem*> SubtitleParser::getSubtitles()
{
    return _subtitles;
}

inline std::string SubtitleParser::getFileData()           //returns whole read file i.e. contents of input.srt
{
    std::ifstream infile(_fileName);
    std::string allData = "";
    std::string line;
    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        allData += line + "\n";
    }
    return allData;

}

inline SubtitleParser::SubtitleParser(void)
{

}

inline SubtitleParser::~SubtitleParser(void)
{
}

//3. SubRipParser class

inline SubRipParser::SubRipParser(void)
{
}

inline void SubRipParser::parse(std::string fileName)      //srt parser
{

    std::ifstream infile(fileName);
    std::string line, start, end, completeLine = "", timeLine = "";
    int subNo, turn = 0;

    /*
     * turn = 0 -> Add subtitle number
     * turn = 1 -> Add string to timeLine
     * turn > 1 -> Add string to completeLine
     */

    while (std::getline(infile, line))
    {
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());

        if (line.compare(""))
        {
            if(!turn)
            {
                subNo=atoi(line.c_str());
                turn++;
                continue;
            }

            if (line.find("-->") != std::string::npos)
            {
                timeLine += line;

                std::vector<std::string> srtTime;
                srtTime = split(timeLine, ' ', srtTime);
                start = srtTime[0];
                end = srtTime[2];

            }
            else
            {
                if (completeLine != "")
                    completeLine += "\n";

                completeLine += line;
            }

            turn++;
        }

        else
        {
            turn = 0;
            _subtitles.push_back(new SubtitleItem(subNo,start,end,completeLine));
            completeLine = timeLine = "";
        }

        if(infile.eof())    //insert last remaining subtitle
        {
            _subtitles.push_back(new SubtitleItem(subNo,start,end,completeLine));
        }
    }
}

inline SubRipParser::SubRipParser(std::string fileName)
{
    _fileName = fileName;
    parse(fileName);
}

inline SubRipParser::~SubRipParser(void)
{
    for(int i=0;i != _subtitles.size();++i)
    {
        if(_subtitles[i])
            delete _subtitles[i];
    }
}

//4. SubtitleItem class

inline SubtitleItem::SubtitleItem(void)
{
}

inline SubtitleItem::SubtitleItem(int subNo, std::string startTime,std::string endTime, std::string text, bool ignore,
                           std::string justDialogue, int speakerCount, int nonDialogueCount,
                           int styleTagCount, int wordCount, std::vector<std::string> speaker, std::vector<std::string> nonDialogue,
                           std::vector<std::string> styleTags, std::vector<std::string> word)
{
    _startTime = timeMSec(startTime);
    _endTime = timeMSec(endTime);
    _text = text;

    _subNo = subNo;
    _startTimeString = startTime;
    _endTimeString = endTime;
    _ignore = ignore;
    _justDialogue = justDialogue;
    _speakerCount = speakerCount;
    _nonDialogueCount = nonDialogueCount;
    _wordCount = wordCount;
    _speaker = speaker;
    _styleTagCount = styleTagCount;
    _styleTag = styleTags;
    _nonDialogue = nonDialogue;
    _word = word;

    extractInfo();
}

inline long int SubtitleItem::timeMSec(std::string value)
{
    std::vector<std::string> t, secs;
    int hours, mins, seconds, milliseconds;

    t = split(value, ':', t);
    hours = atoi(t[0].c_str());
    mins = atoi(t[1].c_str());

    secs = split(t[2], ',', secs);
    seconds = atoi(secs[0].c_str());
    milliseconds = atoi(secs[1].c_str());

    return hours * 3600000 + mins * 60000 + seconds * 1000 + milliseconds;
}

inline long int SubtitleItem::getStartTime() const
{
    return _startTime;
}
inline long int SubtitleItem::getEndTime() const
{
    return _endTime;
}

inline std::string SubtitleItem::getText() const
{
    return _text;
}

inline void SubtitleItem::setStartTime(long int startTime)
{
    _startTime = startTime;
}
inline void SubtitleItem::setEndTime(long int endTime)
{
    _endTime = endTime;
}
inline void SubtitleItem::setText(std::string text)
{
    _text = text;
}
inline void SubtitleItem::setWordTimes(std::vector<long int> wordStartTime, std::vector<long int> wordEndTime, std::vector<long int> wordDuration)
{
    _wordStartTime = wordStartTime;
    _wordEndTime = wordEndTime;
    _wordDuration = wordDuration;
}
inline int SubtitleItem::getSubNo() const
{
    return _subNo;
}
inline std::string SubtitleItem::getStartTimeString() const
{
    return _startTimeString;
}

inline std::string SubtitleItem::getEndTimeString() const
{
    return _endTimeString;
}

inline bool SubtitleItem::getIgnoreStatus() const
{
    if(_ignore)
        return true;

    else
        return false;

}

inline void SubtitleItem::extractInfo(bool keepHTML, bool doNotIgnoreNonDialogues, bool doNotRemoveSpeakerNames)   //process subtitle
{
    std::string output = _text;

    //stripping HTML tags
    if(!keepHTML)
    {
        /*
         * TODO : Before erasing, extract the words.
         * std::vector<std::string> getStyleTags();
         * int getStyleTagCount() const;
         * std::vector<std::string> _styleTag;
         * int _styleTagCount;
         */

        int countP = 0;
        for(char& c : output) // replacing <...> with ~~~~
        {
            if(c=='<')
            {
                countP++;
                c = '~';
            }

            else
            {
                if(countP!=0)
                {
                    if(c != '>')
                        c = '~';

                    else if(c == '>')
                    {
                        c = '~';
                        countP--;
                    }
                }
            }
        }
    }

    //stripping non dialogue data e.g. (applause)

    if(!doNotIgnoreNonDialogues)
    {
        /*
         * TODO : Before erasing, extract the words.
         * std::vector<std::string> getNonDialogueWords();
         * int getNonDialogueCount() const;
         * std::vector<std::string> _nonDialogue;
         * int _nonDialogueCount;
         */

        int countP = 0;
        for(char& c : output)   // replacing (...) with ~~~~
        {
            if(c=='(')
            {
                countP++;
                c = '~';
            }

            else
            {
                if(countP!=0)
                {
                    if(c != ')')
                        c = '~';

                    else if(c == ')')
                    {
                        c = '~';
                        countP--;
                    }
                }
            }
        }
    }

    output.erase(std::remove(output.begin(), output.end(), '~'), output.end()); // deleting all ~

    //Extracting speaker names
    if(!doNotRemoveSpeakerNames)
    {
        for(int i=0; output[i]!='\0';i++)
        {
            int colonIndex = 0, nameBeginIndex = 0;
            if(output[i]==':')  //speaker found; travel back
            {
                _speakerCount++;
                colonIndex = i;

                int tempIndex = 0, foundEvilColon = 0, continueFlag = 0, spaceBeforeColon = 0;

                if(output[i-1] == ' ')
                    spaceBeforeColon = 2;

                /*
                Possible Cases :

                Elon Musk: Hey Saurabh, you are pretty smart.       // First and Last Name
                Saurabh: *_* What? Elon Musk: Yes!                  // Two names in single line
                Saurabh : OMG OMG!                                  // Space before colon
                Elon: LOL World: LAMAO
                Saurabh: ._.                                        // normal

                 */

                for(int j=i - spaceBeforeColon; j>=0;j--)
                {
                    if(output[j] == '.' || output[j] == '!' || output[j] == ',' || output[j] == '?' || output[j] == '\n'
                       || output[j] == ' ' || j== 0)
                    {

                        if(output[j] == '.' || output[j] == '!' || output[j] == ',' || output[j] == '?' || j == 0)
                        {
                            if((continueFlag && j == 0))
                            {
                                if(!isupper(output[j]))
                                {
                                    nameBeginIndex = tempIndex;
                                    break;
                                }

                                else
                                    tempIndex = j;

                            }

                            else if(j!=0)
                                tempIndex = j + 1;
                        }

                        else if(output[j] == ' ' && isupper(output[j+1]))
                        {
                            tempIndex = j;
                            continueFlag = 1;

                            continue;
                        }

                        else if(output[j] == ' ' && !isupper(output[j+1] && tempIndex == 0))
                        {
                            _speakerCount--;
                            foundEvilColon = 1;
                            break;
                        }

                        nameBeginIndex = tempIndex;
                        break;
                    }
                }

                if(foundEvilColon)
                    continue;

                i = nameBeginIndex; //compensating the removal and changes in index

                //check if there's a space after colon i.e. A: Hello vs A:Hello
                int removeSpace = 0;
                if(output[colonIndex + 1]==' ')
                    removeSpace = 1;

                _speaker.push_back(output.substr(nameBeginIndex, colonIndex - nameBeginIndex));
                output.erase(nameBeginIndex, colonIndex - nameBeginIndex + removeSpace);
            }

        }

    }

    // removing more than one whitespaces with one space
    unique_copy (output.begin(), output.end(), std::back_insert_iterator<std::string>(_justDialogue),
                 [](char a,char b)
                 {
                     return isspace(a) && isspace(b);
                 });

    // trimming whitespaces
    const char* whiteSpaces = " \t\n\r\f\v";
    _justDialogue.erase(0, _justDialogue.find_first_not_of(whiteSpaces));
    _justDialogue.erase(_justDialogue.find_last_not_of(whiteSpaces) + 1);

    if(_justDialogue.empty() || _justDialogue == " ")
        _ignore = true;

    else
    {
        _word = split(_justDialogue, ' ', _word); //extracting individual words
        _wordCount = (int)_word.size();
    }
}

inline std::string SubtitleItem::getDialogue(bool keepHTML, bool doNotIgnoreNonDialogues,  bool doNotRemoveSpeakerNames)
{
    if(_justDialogue.empty())
        extractInfo(keepHTML, doNotIgnoreNonDialogues, doNotRemoveSpeakerNames);

    return _justDialogue;
}
inline int SubtitleItem::getSpeakerCount() const
{
    return _speakerCount;
}
inline int SubtitleItem::getNonDialogueCount() const
{
    return _nonDialogueCount;
}
inline int SubtitleItem::getStyleTagCount() const
{
    return _styleTagCount;
}
inline int SubtitleItem::getWordCount() const
{
    return _wordCount;
}
inline std::vector<std::string> SubtitleItem::getSpeakerNames()
{
    return _speaker;
}
inline std::vector<std::string> SubtitleItem::getNonDialogueWords()
{
    return _nonDialogue;
}
inline std::vector<std::string> SubtitleItem::getIndividualWords()
{
    return _word;
}
inline std::string SubtitleItem::getWordByIndex(int index)
{
    return _word[index];
}
inline std::vector<long int> SubtitleItem::getWordStartTimes()
{
    return _wordStartTime;
}
inline std::vector<long int> SubtitleItem::getWordEndTimes()
{
    return _wordEndTime;
}
inline long int SubtitleItem::getWordStartTimeByIndex(int index)
{
    return _wordStartTime[index];
}
inline long int SubtitleItem::getWordEndTimeByIndex(int index)
{
    return _wordEndTime[index];
}
inline std::vector<std::string> SubtitleItem::getStyleTags()
{
    return _styleTag;
}
inline SubtitleItem::~SubtitleItem(void)
{

}

//5. SubtitleWordclass

inline SubtitleWord::SubtitleWord(void)
{
    _text = "";
}

inline SubtitleWord::SubtitleWord(std::string text)
{
    _text = text;
}

inline std::string SubtitleWord::getText() const
{
    return _text;
}

inline SubtitleWord::~SubtitleWord(void)
{
}


#endif //SRTPARSER_H