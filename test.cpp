#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <stack>
#include <algorithm>

// Check if two strings are anagrams (contain the same characters in any order).
// Ex: "hello" and "olleh" are anagrams
bool isAnagram(std::string s, std::string t)
{
    if(s.size() == t.size())
    {
        int i = 0;
        int j = s.size() - 1;
        while (i < j){
            if (s[i] != t[j])
                return false;
            i++;
            j--;   
        }
        return true;
    }
    return false;
}

//Function Frequency counter
int majorityElement(std::vector<int>& nums)
{
    std::unordered_map<int, int> count;
    int size = nums.size();
    for(auto num : nums)
    {
        count[num]++;
        if(count[num] > size / 2)
            return num;
    }
    return -1;
}

//Test Valid parentheses
bool isValid(std::string s)
{
    std::unordered_map<char, char> brackets = {
        {')', '('},
        {']', '['},
        {'}', '{'}
    };
    std::vector<char> stack;
    for(auto c : s)
    {
        if (brackets.count(c))
        {
            if (stack.empty() || stack.back() != brackets[c])
                return false;
            stack.pop_back();
        }
        else
            stack.push_back(c);
    }
    return true;
}

//Get longest substring without repeating characters
int lengthOfLongestSubstring(std::string s)
{
    std::string current = "";
    int maxlen = 0;
    for(char c : s)
    {
        size_t pos = current.find(c);
        if (pos != std::string::npos)
            current = current.substr(pos + 1);
        current += c;
        maxlen = std::max(maxlen, static_cast<int>(current.size()));
    }
    return maxlen;
}

// Given a vector of integers and a target, 
// return indices of two numbers that add up to the target.
std::vector<int> twoSum(std::vector<int>& nums, int target)
{
    std::vector<int> ret;
    for(int i = 0; i < static_cast<int>(nums.size()); ++i) {
        for(int j = i + 1; j < static_cast<int>(nums.size()); ++j) {
            if (nums[i] + nums[j] == target)
                return{i, j};
        }
    }
    return {};
}

// Find the length of the longest palindrome that can be built with the letters of a given string.
// longest palindrome
// int longestPalidrome(std::string s)
// {
//     int i = 0;
//     int j = s.size() - 1;
//     while (i < s.size())
//     {
//         while (j > i)
//         {
//             if(s[i] == s[j])
//                 return false;
//             j--;    
//         }
//         i++;
//     }
//     return true;    
// }

int main()
{
    std::vector<int> nums = {3, 5, 3, 7, 3, 8, 3, 3, 3, 3};
    int result = majorityElement(nums);

    std::cout << "Majority Element: " << result << std::endl;
    std::string s = "({[]})";
    std::string s1 = "{[]})";
    bool valid = isValid(s);
    bool valid1 = isValid(s1);
    std::cout << "Is valid parentheses: " << valid << std::endl;
    std::cout << "Is invalid parentheses: " << valid1 << std::endl;
    std::cout << "abcabcbb" << lengthOfLongestSubstring("abcabcbb") << "\n";
    std::cout << "pwwkew" << lengthOfLongestSubstring("pwwkew") << "\n";
    std::cout << "twosum" << std::endl;
    std::vector<int> arr = {1, 2, 3, 5, 6, 7, 4};
    std::vector res = twoSum(arr, 11);
    for(auto i = res.begin(); i != res.end(); ++i)
        std::cout << *i << " " << std::endl;
    std::cout << "hello and olleh are anagram " << isAnagram("hello", "olleh") << std::endl;
    std::cout << "month and htnmo are anagram " << isAnagram("month", "htnmo") << std::endl;
    // return 0;
}