#include <iostream>     //����ͷ�ļ�  
#include <stdlib.h>  
#include <new>
#include <memory>
#include <functional>
#include <array>
#include <vector>

using namespace std;

double fuc(double x, double y) //���庯��  
{  
    if(y==0)  
    {  
        throw y;     //����Ϊ0���׳��쳣  
    }  
    return x/y;     //���򷵻�����������  
}  
class foo
{
public:
	foo(){};
	void test_foo(const foo & f)
	{
		c = f.c;
	}
private:
	int c; 
};

class Complex 
{         

private :
        double    m_real;
        double    m_imag;

public:

        //    �޲������캯��
        // �������һ������û��д�κι��캯��,��ϵͳ���Զ�����Ĭ�ϵ��޲ι��캯��������Ϊ�գ�ʲô������
        // ֻҪ��д��һ�������ĳһ�ֹ��캯����ϵͳ�Ͳ������Զ���������һ��Ĭ�ϵĹ��캯�������ϣ����һ���������޲ι��캯��������Ҫ�Լ���ʾ��д����
        Complex(void)
        {
			 std::cout << "ah" << std::endl;
             m_real = 0.0;
             m_imag = 0.0;
        } 
        
        //    һ�㹹�캯����Ҳ�����ع��캯����
        // һ�㹹�캯�������и��ֲ�����ʽ,һ��������ж��һ�㹹�캯����ǰ���ǲ����ĸ����������Ͳ�ͬ������c++�����غ���ԭ��
        // ���磺�㻹����дһ�� Complex( int num)�Ĺ��캯������
        // ��������ʱ���ݴ���Ĳ�����ͬ���ò�ͬ�Ĺ��캯��
        Complex(double real, double imag)
        {
             m_real = real;
             m_imag = imag;         
         }
        
        //    ���ƹ��캯����Ҳ��Ϊ�������캯����
        //    ���ƹ��캯������Ϊ�����������ã����ڸ���һ���Ѵ��ڵĶ����Ƴ�һ���µĸ���Ķ���һ���ں����лὫ�Ѵ��ڶ�������ݳ�Ա��ֵ����һ�ݵ��´����Ķ�����
        //    ��û����ʾ��д���ƹ��캯������ϵͳ��Ĭ�ϴ���һ�����ƹ��캯��������������ָ���Աʱ����ϵͳĬ�ϴ����ø��ƹ��캯������ڷ��գ�����ԭ�����ѯ �й� ��ǳ������ �������������������
        Complex(const Complex & c)
        {
			std::cout << "eh" << std::endl;
                // ������c�е����ݳ�Աֵ���ƹ���
                m_real = c.m_real;
                m_imag = c.m_imag;
        }            
    
        // ����ת�����캯��������һ��ָ�������͵Ķ��󴴽�һ������Ķ���
        // ���磺���潫����һ��double���͵Ķ��󴴽���һ��Complex����
        Complex::Complex(double r)
        {
                m_real = r;
                m_imag = 0.0;
        }

        // �Ⱥ����������
        // ע�⣬������Ƹ��ƹ��캯������=�ұߵı�������ֵ���Ƹ��Ⱥ���ߵĶ����������ڹ��캯�����Ⱥ��������ߵĶ�������Ѿ�������
        // ��û����ʾ��д=��������أ���ϵͳҲ�ᴴ��һ��Ĭ�ϵ�=��������أ�ֻ��һЩ�����Ŀ�������
        Complex &operator=( const Complex &rhs )
        {
				std::cout << "hheh" << std::endl;
                // ���ȼ��Ⱥ��ұߵ��Ƿ������ߵĶ��󱾣����Ǳ�������,��ֱ�ӷ���
                if ( this == &rhs ) 
                {
                        return *this;
                }
                
                // ���ƵȺ��ұߵĳ�Ա����ߵĶ�����
                this->m_real = rhs.m_real;
                this->m_imag = rhs.m_imag;
                
               // �ѵȺ���ߵĶ����ٴδ���
               // Ŀ����Ϊ��֧������ eg:    a=b=c ϵͳ�������� b=c
               // Ȼ������ a= ( b=c�ķ���ֵ,����Ӧ���Ǹ���cֵ���b����)    
                return *this;
        }

};

Complex test1(const Complex& c)
{
   return c;
}
  
Complex test2(const Complex c)
{
   return c;
}
   
Complex test3()
{
   static Complex c(1.0,5.0);
   return c;
}
  
Complex& test4()
{
   static Complex c(1.0,5.0);
   return c;
}
class C
{
public:
	int add(int a, int b)
	{
		return addd(a,b);
	}
private:
	virtual int addd(int a, int b)
	{
		return a + b;
	}
};

class B :public C
{
public:
	int addd(int a, int b)
	{
		return a - b;
	}
private:
	
};
void main()  
{    
	C* character = new B;
	std::cout << character->add(5,2) << std::endl; 

	std::array<int,10> a = {1,2,3,6,3,7,8,0,4,9};
	 
	std::function<bool (const int&, const int&)> func = [](const int &a, const  int &b)->bool{ return a > b;};
      
	std::sort(a.begin(),a.end(), func);

	for(std::size_t i = 0; i < a.size(); i++)
		std::cout << a[i] << " ";
	std::cout << std::endl;
  
	//Complex a;
	//Complex c;
	//test2(a);
	test4();
	 
    double res;  
    try  //�����쳣  
    {  
        res=fuc(2,3);  
        cout<<"The result of x/y is : "<<res<<endl;  
      //  res=fuc(4,0); //�����쳣�������ڲ����׳��쳣  
		std::cout << res << std::endl;
    }  
    catch(double y)             //���񲢴����쳣  
    {  
		 std::cout << y << std::endl;
         cerr<<"error of dividing zero.\n";  
         exit(1);                //�쳣�˳�����  
    }  
	system("pause");
} 