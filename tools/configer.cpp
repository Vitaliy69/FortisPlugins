/*!
    \file configer.cpp
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#include "configer.h"

Configer::Configer(std::string _confFileName)
{
    try
    {
        m_fl.open(_confFileName.c_str(), std::ios::binary);
    }
    catch (std::ifstream::failure)
    {
        std::cerr << "Error file open!\n";
    }
}

Configer::~Configer()
{
    if (m_fl.is_open())
        m_fl.close();
}

std::string Configer::getStrValue(std::string _section, std::string _param, bool _saveSpace)
{
    if (m_fl.is_open())
    {
        char buf[MAX_BUF_SIZE];
        bool sect = false;

        m_fl.clear();
        m_fl.seekg(0);

        while(m_fl.getline(buf, sizeof buf))
        {
            std::string rdStr(buf);

            size_t com = rdStr.find("#");
            if (com != std::string::npos)
                rdStr = rdStr.substr(0, com);

            if ((rdStr.find("[") != std::string::npos) || (rdStr.find("]") != std::string::npos))
            {
                sect = false;
                rdStr.erase(std::remove(rdStr.begin(), rdStr.end(), ' '), rdStr.end());
            }

            std::string par = "[" + _section + "]";
            if(rdStr.find(par) != std::string::npos)
                sect = true;

            if (rdStr.find("=") == std::string::npos)
                continue;

            std::string sep = rdStr.substr(0, rdStr.find("="));
            std::string remBack = sep;

            remBack.erase(std::remove(remBack.begin(), remBack.end(), '\t'), remBack.end());
            remBack.erase(std::remove(remBack.begin(), remBack.end(), '\n'), remBack.end());
            remBack.erase(std::remove(remBack.begin(), remBack.end(), ' '), remBack.end());

            if (sect && rdStr.find(sep) != std::string::npos && remBack == _param)
            {
                std::string val = rdStr.substr(rdStr.find("=") + 1, rdStr.length() - rdStr.find("=") - 1);

                short shift = 0;
                for (unsigned short i = 0; i < val.length(); i++)
                {
                    if (val[i] == ' ')
                        shift = i + 1;
                    else
                        break;
                }

                val = val.substr(shift, val.length() - shift);

                if (!_saveSpace)
                    val.erase(std::remove(val.begin(), val.end(), ' '), val.end());

                val.erase(std::remove(val.begin(), val.end(), '\r'), val.end());
                val.erase(std::remove(val.begin(), val.end(), '\n'), val.end());
                val.erase(std::remove(val.begin(), val.end(), '\t'), val.end());

                return val;
            }
        }
    }

    return "";
}

int Configer::getIntValue(std::string _section, std::string _param)
{
    std::string value = getStrValue(_section, _param);

    try
    {
        if (!value.empty())
            return abs(atoi(value.c_str()));
        else
            return 0;
    }
    catch(...) {}

    return 0;
}

bool Configer::getBoolValue(std::string _section, std::string _param)
{
    std::string value = getStrValue(_section, _param);

    if (!value.empty())
    {
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        if (value == "true" || value == "1" || value == "yes")
            return true;
        else
            return false;
    }
    else
        return false;
}

bool Configer::validateColor(std::string _color)
{
    if (_color.length() != 10 || _color[0] != '0' || _color[1] != 'x')
        return false;
    else
        return true;
}
