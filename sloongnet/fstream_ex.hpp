#include <string>
#include <fstream>
#include <iostream>

using namespace std;

class fstream_ex 
{
public:
	static bool read_all(const string& path, string& data) 
	{
		ifstream f(path);
		if (f.is_open())
		{
			f.seekg(0, f.end);
			int length = f.tellg();
			f.seekg(0, f.beg);

			char* buffer = new char[length];
			f.read(buffer, length);
			data = string(buffer, length);
			delete[] buffer;
			f.close();
			return true;
		}
		return false;
	}

	static bool write_all(const string& path, const string& data)
	{
		ofstream f(path);
		if (f.is_open())
		{
			f.write(data.data(), data.length());
			f.flush();
			f.close();
			return true;
		}
		return false;
	}

};