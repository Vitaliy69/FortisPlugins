/*!
    \file configer.cpp
    \author Vitaliy Gribko
    
    Distributed under the GPL v2
    see the file LICENSE for details
    or visit http://www.gnu.org/copyleft/gpl.html
*/

#ifndef _CONFIGER_H_
#define _CONFIGER_H_

#include <iostream>
#include <fstream>
#include <algorithm>

#include <string.h>
#include <stdlib.h>

/// Configer
/*!
    \class Configer
    \brief Class for configuration files reading
*/

#define MAX_BUF_SIZE            255

class Configer
{
    
public:
    /// Constructor
    /*!
        \param std::string _confFileName Full path to configuration file
    */
    explicit Configer(std::string _confFileName);

    virtual ~Configer();

    /// Get string value
    /*!
        \param std::string _section Section name
        \param std::string _param Parameter name
        \param bool _saveSpace Save all spaces in output string for true
    */
    std::string getStrValue(std::string _section, std::string _param, bool _saveSpace = false);
    
    /// Get integer value
    /*!
        \param std::string _section Section name
        \param std::string _param Parameter name
    */
    int getIntValue(std::string _section, std::string _param);
    
    /// Get boolean value
    /*!
        \param std::string _section Section name
        \param std::string _param Parameter name
    */
    bool getBoolValue(std::string _section, std::string _param);
    
    /// Check color validity for Fortis API
    /*!
        \param string _color String encoding value
        \return True for valide color
    */
    bool validateColor(std::string _color);

private:
    std::ifstream m_fl;
    std::string m_confFileName;
};

#endif
