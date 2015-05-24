
#include "UserInput.h"

#include <iostream>
#include <limits.h>

using namespace std;

inline void clearInput()
{
    cin.clear();
    cin.ignore(INT_MAX, '\n');
}

inline void printVector(vector<int> v, bool withIndex)
{
    if(!withIndex)
    {
        cout << "(";
    }
    for (unsigned int i = 0; i < v.size(); i++)
    {
        if(withIndex)
        {
            cout << endl << i << ": ";
        }
        else if(i > 0)
        {
            cout << ", ";
        }
        cout << v[i];
        
    }
    if(!withIndex)
    {
        cout << ")";
    }
    else
    {
        cout << endl;
    }
}

inline void printVector(vector<string> v, bool withIndex)
{
    if(!withIndex)
    {
        cout << "(";
    }
    for (unsigned int i = 0; i < v.size(); i++)
    {
        if(withIndex)
        {
            cout << endl << i << ": ";
        }
        else if(i > 0)
        {
            cout << ", ";
        }
        cout << v[i];
        
    }
    if(!withIndex)
    {
        cout << ")";
    }
    else
    {
        cout << endl;
    }
}

bool inputBoolean(std::string message)
{
    cout << message << " (Yes/No): ";
    string result;
    cin >> result;
    if(result == "Y" || result== "Yes" || result == "y" || result == "yes")
    {
        return true;
    }
    if(result == "N" || result== "No" || result == "n" || result == "no")
    {
        return false;
    }
    cout << "Invalid input: Please type 'Yes' or 'No'!" << endl;
    return inputBoolean(message);
}

string inputString(string message)
{
    cout << message << ": ";
    string result;
    cin >> result;
    return result;
}

string selectOption(string message, vector<string> options, string defaultOption)
{
    cout << message;
    printVector(options, false);
    cout << " [" << defaultOption << "]";
    cout << ": ";
    string result;
    cin >> result;
    for (size_t i = 0; i < options.size(); i++)
    {
        if(result == options[i])
        {
            return options[i];
        }
    }
    return defaultOption;
}

unsigned int selectOptionIndex(std::string message, std::vector<std::string> options, unsigned int defaultIndex)
{
    cout << message;
    cout << " [" << defaultIndex << "]";
    cout << ": ";
    printVector(options, true);
    unsigned int selectedIndex;
    if(!(cin >> selectedIndex))
    {
        clearInput();
        cout << "Invalid input: You must input a number!" << endl;
        return selectOptionIndex(message, options, defaultIndex);
    }
    if(selectedIndex >= 0 && selectedIndex < options.size())
    {
        return selectedIndex;
    }
    return defaultIndex;
}

int inputNumber(string message, bool allowZero, bool allowNegative)
{
    cout << message << ": ";
    int result;
    if(!(cin >> result))
    {
        clearInput();
        cout << "Invalid input: You must input a number!" << endl;
        return inputNumber(message, allowZero, allowNegative);
    }
    if(!allowZero && result == 0)
    {
        cout << "Invalid input: zero is not allowed!" << endl;
        return inputNumber(message,allowZero,allowNegative);
    }
    if(!allowNegative && result < 0)
    {
        cout << "Invalid input: only positive numbers are allowed!" << endl;
        return inputNumber(message, allowZero, allowNegative);
    }
    return result;
}

int selectOption(std::string message, std::vector<int> options, int defaultOption)
{
    cout << message;
    printVector(options, false);
    cout << " [" << defaultOption << "]";
    cout << ": ";
    int result;
    if(!(cin >> result))
    {
        clearInput();
        cout << "Invalid input: You must input a number!" << endl;
        return selectOption(message, options, defaultOption);
    }
    for (size_t i = 0; i < options.size(); i++)
    {
        if(result == options[i])
        {
            return options[i];
        }
    }
    return defaultOption;
}

unsigned int selectOptionIndex(std::string message, std::vector<int> options, unsigned int defaultIndex)
{
    cout << message;
    cout << " [" << defaultIndex << "]";
    cout << ": ";
    printVector(options, true);
    unsigned int selectedIndex;
    if(!(cin >> selectedIndex))
    {
        clearInput();
        cout << "Invalid input: You must input a number!" << endl;
        return selectOptionIndex(message, options, defaultIndex);
    }
    if(selectedIndex >= 0 && selectedIndex < options.size())
    {
        return selectedIndex;
    }
    return defaultIndex;
}