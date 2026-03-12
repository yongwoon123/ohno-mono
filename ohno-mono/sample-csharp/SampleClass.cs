using System;
using System.Runtime.CompilerServices;

// -------------------------------------------------------
// Shared utilities (all chapters)
// -------------------------------------------------------

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

    public Vector3(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    public override string ToString() => $"Vec3({x:F2}, {y:F2}, {z:F2})";
}

// -------------------------------------------------------
// Chapter 1 — Entity
// -------------------------------------------------------

public class Entity
{
    public int Health = 100;

    public Entity() { Out.Log("Entity default constructor called"); }
    public Entity(int id) { Out.Log("Entity constructor called with id: " + id); }

    public void SampleVoidMethod() { Out.Log("SampleVoidMethod called"); }
    public void Move(Vector3 v) { Out.Log("Move called with " + v); }

    public void TriggerCallback()
    {
        Out.Log("Calling back into C++...");
        Internal_Callback();
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_Callback();
}

// -------------------------------------------------------
// Chapter 2 — Hot Reload
// -------------------------------------------------------

public class ScriptA
{
    public void Hello()
    {
        Out.Log("Hello from ScriptA - version 1");
    }

    public void TriggerCallback()
    {
        Out.Log("Calling back into C++...");
        Internal_Callback();
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_Callback();
}

// -------------------------------------------------------
// Chapter 3 — GC Handles
// -------------------------------------------------------

public class GCTestObject
{
    public void Ping()
    {
        Out.Log("Method called successfully on GC-pinned object");
    }
}

// -------------------------------------------------------
// Chapter 4 — Script Component Pattern
// -------------------------------------------------------

public abstract class SimpleBehaviour
{
    public bool isActive = true;
    internal bool hasAwake = false;
    internal bool hasStart = false;

    public virtual void Awake()       { }
    public virtual void Start()       { }
    public virtual void Update()      { }
    public virtual void LateUpdate()  { }
    public virtual void FixedUpdate() { }
    public virtual void OnDestroy()   { }
}

public class PlayerController : SimpleBehaviour
{
    private int tick = 0;

    public override void Awake()  { Out.Log("[PlayerController] Awake"); }
    public override void Start()  { Out.Log("[PlayerController] Start"); }
    public override void Update() { tick++; Out.Log("[PlayerController] Update - tick " + tick); }
}

public class EnemyAI : SimpleBehaviour
{
    private int tick = 0;

    public override void Awake()  { Out.Log("[EnemyAI] Awake"); }
    public override void Start()  { Out.Log("[EnemyAI] Start"); }
    public override void Update() { tick++; Out.Log("[EnemyAI] Update - tick " + tick); }
}

public class CameraFollow : SimpleBehaviour
{
    private int tick = 0;

    public override void Awake()  { Out.Log("[CameraFollow] Awake"); }
    public override void Start()  { Out.Log("[CameraFollow] Start"); }
    public override void Update() { tick++; Out.Log("[CameraFollow] Update - tick " + tick); }
}

// -------------------------------------------------------
// Chapter 5 - C# Properties Backed by Internal Calls
// -------------------------------------------------------

public class MyTransform
{
    private uint goId;

    // Properties delegate to internal calls.
    // From the caller's perspective, 'position' and 'isActive' look like normal fields.
    // Under the hood every get and set crosses the C++/C# boundary via the registered
    // internal call functions.
    public Vector3 position
    {
        get => Internal_GetPosition(goId);
        set => Internal_SetPosition(goId, value);
    }

    public bool isActive
    {
        get => Internal_GetIsActive(goId);
        set => Internal_SetIsActive(goId, value);
    }

    public MyTransform(uint id) { goId = id; }

    public void PrintPosition()
    {
        Out.Log("transform.position = " + position);
    }

    public void SetPositionTo(Vector3 v)
    {
        Out.Log("Setting position to " + v);
        position = v;
    }

    public void PrintIsActive()
    {
        Out.Log("isActive = " + isActive.ToString().ToLower());
    }

    public void SetIsActiveTo(bool val)
    {
        Out.Log("Setting isActive to " + val.ToString().ToLower());
        isActive = val;
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 Internal_GetPosition(uint id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_SetPosition(uint id, Vector3 pos);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool Internal_GetIsActive(uint id);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_SetIsActive(uint id, bool val);
}

// -------------------------------------------------------
// Chapter 6 - String Marshalling
// -------------------------------------------------------

public class StringBridge
{
    // C++ to C#: call an internal that returns a string, then print it.
    // Mono automatically marshals the MonoString* return value as System.String.
    public void PrintName()
    {
        string name = Internal_GetName();
        Out.Log("Received name = \"" + name + "\"");
    }

    // C# to C++: build a string on the C# side and pass it to an internal call.
    // Mono marshals the System.String argument as MonoString* on the C++ side.
    public void SendString()
    {
        string msg = "hello from C#";
        Out.Log("Passing \"" + msg + "\" to C++");
        Internal_ReceiveString(msg);
    }

    // Round-trip: get a name from C++, append a suffix, pass the result back.
    public void RoundTrip()
    {
        string name = Internal_GetOriginalName();
        Out.Log("Appending suffix...");
        string modified = name + "_modified";
        Out.Log("Passing back \"" + modified + "\"");
        Internal_ReceiveModifiedName(modified);
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern string Internal_GetName();

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_ReceiveString(string msg);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern string Internal_GetOriginalName();

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_ReceiveModifiedName(string name);
}

// -------------------------------------------------------
// Chapter 7 - OhnoArray: Sending Arrays (C++ -> C#)
// -------------------------------------------------------

public class ArrayReceiver
{
    // C++ builds an OhnoArray<int> and passes the raw MonoArray* via internal call args.
    // On the C# side it arrives as a normal int[] - no special handling needed.
    public void ReceiveIntArray(int[] arr)
    {
        Out.Log("Received array, length = " + arr.Length);
        for (int i = 0; i < arr.Length; i++)
            Out.Log("[" + i + "] = " + arr[i]);
    }

    // Strings arrive as a managed string[] - each element was converted by
    // MonoHelper::StringToMono and stored via mono_array_set on the C++ side.
    public void ReceiveStringArray(string[] arr)
    {
        Out.Log("Received array, length = " + arr.Length);
        for (int i = 0; i < arr.Length; i++)
            Out.Log("[" + i + "] = " + arr[i]);
    }

    // Structs arrive as a Vector3[] - the C++ Vector3 layout matches this struct
    // exactly (three floats, same order), so the memcpy bulk copy is valid.
    public void ReceiveVectorArray(Vector3[] arr)
    {
        Out.Log("Received array, length = " + arr.Length);
        for (int i = 0; i < arr.Length; i++)
            Out.Log("[" + i + "] = " + arr[i]);
    }
}

// -------------------------------------------------------
// Chapter 8 - OhnoArray: Receiving Arrays (C# -> C++)
// -------------------------------------------------------

public class ArraySender
{
    // C++ calls these methods and receives the return value as ::MonoObject*.
    // It casts to ::MonoArray* and wraps in OhnoArray(::MonoArray*) to read back.

    public int[] GetIntArray()
    {
        Out.Log("Returning int[] { 10, 20, 30, 40, 50 }");
        return new int[] { 10, 20, 30, 40, 50 };
    }

    public Vector3[] GetVectorArray()
    {
        Vector3 a = new Vector3(0, 1, 2);
        Vector3 b = new Vector3(10, 20, 30);
        Out.Log($"Returning Vector3[] {{ {a}, {b} }}");
        return new Vector3[] { a, b };
    }

    public string[] GetStringArray()
    {
        Out.Log("Returning string[] { \"alpha\", \"beta\", \"gamma\" }");
        return new string[] { "alpha", "beta", "gamma" };
    }
}

public class HasArrayField
{
    private int[] Numbers = new int[] { 1, 2, 3 };

    public HasArrayField() { Out.Log("Numbers: {1, 2, 3}"); }
}
