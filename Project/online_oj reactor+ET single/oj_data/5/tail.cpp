// tail.cpp不给用户看的,最终编译的时候，
///会把用户提交的代码和tail.cpp合并成一个文件再进行编译


class Test
{
    public:
    int add(int a, int b)
    {
        return a + b;
    }
}

void Test1()
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
    
    Solution user;
    Test own;

    if (user.add(10, 10) == own.add(10, 10))
    {
        cout << "Pass all test cases!" << endl;
    }
    else
    {
        cout << "Error!" << endl;
    }
};
