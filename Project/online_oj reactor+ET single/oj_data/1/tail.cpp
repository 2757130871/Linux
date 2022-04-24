// tail.cpp不给用户看的,最终编译的时候，
///会把用户提交的代码和tail.cpp合并成一个文件再进行编译

class Solution_
{
public:
    bool isPalindrome(int x)
    {
        return true;
    }
};

void Test1(int &count)
{
}

void Test2()
{
}

void Test3()
{
}

void Test4()
{
}

int main()
{
    int count = 0;
    Solution user;
    Solution_ own;

    if (user.isPalindrome(1) == own.isPalindrome(1))
    {
        cout << "Pass all test cases!" << endl;
    }
    else
    {
        cout << "Error!" << endl;
    }
};
