extern "C"
{
	double HelloWorld(double b, int& i)
	{
	    static char* str = "abcdefghijklmnopqrstuvwxyz";
	    //s = str;
	    i = 25;
	    return 1.2 * b;
	}
}
