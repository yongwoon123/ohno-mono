using System;
using System.Runtime.CompilerServices;

class SampleClass
{
	public int SampleInt = 20;
	public float SampleFloat = 50.0f;

	public void SampleFunctionA()
	{
		Console.WriteLine("Sameple Function A");
	}

	public int SampleFunctionB(int a, float c, string d, uint e)
	{
		return 5;
	}
}

public class Out
{
	public static void Log<T>(T val)
	{
		Console.WriteLine("C#: " + val);
	}
}

public struct Vector3
{
	public float x, y, z; 
	public override string ToString() => $"Vec3({x}, {y}, {z})";
}

public class A
{
	public int BaseValueIs20 = 20;
	public float BaseValueIs10F = 10.0f;

	public A()
	{
		Out.Log("Default ctor");
	}

	public A(int b)
	{
		Out.Log("Conversion ctor IN:" + b);
	}

	public void SampleFunctionC()
	{
		Out.Log("This is Sample Function C");
	}

	public void SampleFunctionD(Vector3 b)
	{
		Out.Log("This is Sample Function D " + b);
	}

	public void SampleFunctionE()
	{
		Out.Log("This is Sample Function E");
		Internal_CallCPPFunc();
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void Internal_CallCPPFunc();
}
