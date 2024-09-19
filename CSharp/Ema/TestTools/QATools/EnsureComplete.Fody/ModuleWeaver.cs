/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using Fody;
using Mono.Cecil;
using Mono.Cecil.Cil;
using static Mono.Cecil.Cil.Code;

namespace LSEG.Ema.Access.EnsureComplete;

/// <summary>
/// This internal code analyser emits warning messages when Complete() is not called on ComplexTypes in Series* assemblies 
/// ("Attempt to add a ComplexType while Complete() was not called on this ComplexType.")
/// </summary>
/// <remarks>
/// To enable the analyser when building use "dotnet build -property:EnsureComplete=true QATools.sln"
/// </remarks>
public class ModuleWeaver : BaseModuleWeaver
{
    private static readonly Type[] requiresCompleteTypes = new[] {
        typeof(ElementList),
        typeof(Map),
        typeof(FieldList),
        typeof(FilterList),
        typeof(OmmArray),
        typeof(Vector),
        typeof(Series)};

    public override void Execute() => Validate(ModuleDefinition.GetTypes().ToArray());

    public override bool ShouldCleanReference => true;

    public override IEnumerable<string> GetAssembliesForScanning() => Enumerable.Empty<string>();

    private static SequencePoint? FindSequencePoint(MethodDebugInformation debugInfo, Instruction? instruction)
    {
        SequencePoint? seqPoint;
        do
        {
            seqPoint = debugInfo.GetSequencePoint(instruction);
            instruction = instruction?.Previous;
        } while (seqPoint is null & instruction is not null);
        return seqPoint;
    }

    private class Completable
    {
        public bool Completed { get; set; }
    }

    private void Validate(TypeDefinition[] allTypes)
    {
        foreach (var type in allTypes)
        {
            foreach (var method in type.Methods)
            {
                if (method.HasBody)
                    try
                    {
                        Validate(method);
                    }
                    catch (Exception ex)
                    {
                        WriteInfo(ex.Message);
                    }
            }
        }
    }

    private void Validate(MethodDefinition method)
    {
        Dictionary<int, Completable?> localVariables = new();
        Stack<Completable?> evaluationStack = new();

        void Stloc(int index) => localVariables[index] = evaluationStack.Count > 0 ? evaluationStack.Pop() : null;
        void Ldloc(int index) => evaluationStack.Push(localVariables[index]);
        void LdNull() => evaluationStack.Push(null);
        void Pop()
        {
            if (evaluationStack.Count > 0) evaluationStack.Pop();
        }

        bool IsCompletable(TypeReference refType) => requiresCompleteTypes.Select(t => t.FullName).Contains(refType.FullName);

        foreach (var il in method.Body.Instructions)
        {
            void WriteIncompleteWarning() =>
                WriteWarning("Attempt to add a ComplexType while Complete() was not called on this ComplexType.", FindSequencePoint(method.DebugInformation, il));

            var completable = il.Operand is MethodReference mr && IsCompletable(mr.DeclaringType);
            var completeCall = il.Operand is MethodReference methodRef && methodRef.Name == "Complete";
            if (il.Operand is MethodReference m)
            {
                if (il.OpCode.Code == Callvirt || il.OpCode.Code == Call)
                {
                    if (!completeCall && evaluationStack.Take(m.Parameters.Count).Where(c => c is not null).Any(c => !c!.Completed))
                        WriteIncompleteWarning();
                    if (completable)
                        evaluationStack.Skip(m.Parameters.Count).First(v => v is not null)!.Completed = completeCall;
                    for (var i = 0; i < m.Parameters.Count; i++)
                        evaluationStack.Pop();
                    if (evaluationStack.Count > 0)
                    {
                        if (IsCompletable(m.ReturnType) && evaluationStack.Peek() is Completable comp)
                        {
                            evaluationStack.Push(comp);
                        }
                        else
                        {
                            LdNull();
                        }
                    }
                }
            }

            switch (il.OpCode.Code)
            {
                case Ldarg_0:
                    LdNull();
                    break;

                case Ldarg_1:
                    LdNull();
                    break;

                case Ldarg_2:
                    LdNull();
                    break;

                case Ldarg_3:
                    LdNull();
                    break;

                case Ldloc_0:
                    Ldloc(0);
                    break;

                case Ldloc_1:
                    Ldloc(1);
                    break;

                case Ldloc_2:
                    Ldloc(2);
                    break;

                case Ldloc_3:
                    Ldloc(3);
                    break;

                case Stloc_0:
                    Stloc(0);
                    break;

                case Stloc_1:
                    Stloc(1);
                    break;

                case Stloc_2:
                    Stloc(2);
                    break;

                case Stloc_3:
                    Stloc(3);
                    break;

                case Ldarg_S:
                    LdNull();
                    break;

                case Ldarga_S:
                    LdNull();
                    break;

                case Ldloc_S:
                    Ldloc(((VariableDefinition)il.Operand).Index);
                    break;

                case Ldloca_S:
                    LdNull();
                    break;

                case Ldnull:
                    LdNull();
                    break;

                case Ldc_I4_M1:
                    LdNull();
                    break;

                case Ldc_I4_0:
                    LdNull();
                    break;

                case Ldc_I4_1:
                    LdNull();
                    break;

                case Ldc_I4_2:
                    LdNull();
                    break;

                case Ldc_I4_3:
                    LdNull();
                    break;

                case Ldc_I4_4:
                    LdNull();
                    break;

                case Ldc_I4_5:
                    LdNull();
                    break;

                case Ldc_I4_6:
                    LdNull();
                    break;

                case Ldc_I4_7:
                    LdNull();
                    break;

                case Ldc_I4_8:
                    LdNull();
                    break;

                case Ldc_I4_S:
                    LdNull();
                    break;

                case Ldc_I4:
                    LdNull();
                    break;

                case Ldc_I8:
                    LdNull();
                    break;

                case Ldc_R4:
                    LdNull();
                    break;

                case Ldc_R8:
                    LdNull();
                    break;

                case Code.Pop:
                    Pop();
                    break;

                case Ldind_I1:
                    LdNull();
                    break;

                case Ldind_U1:
                    LdNull();
                    break;

                case Ldind_I2:
                    LdNull();
                    break;

                case Ldind_U2:
                    LdNull();
                    break;

                case Ldind_I4:
                    LdNull();
                    break;

                case Ldind_U4:
                    LdNull();
                    break;

                case Ldind_I8:
                    LdNull();
                    break;

                case Ldind_I:
                    LdNull();
                    break;

                case Ldind_R4:
                    LdNull();
                    break;

                case Ldind_R8:
                    LdNull();
                    break;

                case Ldind_Ref:
                    LdNull();
                    break;

                case Ldobj:
                    LdNull();
                    break;

                case Ldstr:
                    LdNull();
                    break;

                case Newobj:
                    evaluationStack.Push(completable ? new() : null);
                    break;

                case Ldfld:
                    LdNull();
                    break;

                case Ldflda:
                    LdNull();
                    break;

                case Stfld:
                    break;

                case Ldsfld:
                    LdNull();
                    break;

                case Ldsflda:
                    LdNull();
                    break;

                case Ldlen:
                    LdNull();
                    break;

                case Ldelema:
                    LdNull();
                    break;

                case Ldelem_I1:
                    LdNull();
                    break;

                case Ldelem_U1:
                    LdNull();
                    break;

                case Ldelem_I2:
                    LdNull();
                    break;

                case Ldelem_U2:
                    LdNull();
                    break;

                case Ldelem_I4:
                    LdNull();
                    break;

                case Ldelem_U4:
                    LdNull();
                    break;

                case Ldelem_I8:
                    LdNull();
                    break;

                case Ldelem_I:
                    LdNull();
                    break;

                case Ldelem_R4:
                    LdNull();
                    break;

                case Ldelem_R8:
                    LdNull();
                    break;

                case Ldelem_Ref:
                    LdNull();
                    break;

                case Ldelem_Any:
                    LdNull();
                    break;

                case Ldtoken:
                    LdNull();
                    break;

                case Ldftn:
                    LdNull();
                    break;

                case Ldvirtftn:
                    LdNull();
                    break;

                case Ldarg:
                    LdNull();
                    break;

                case Ldarga:
                    LdNull();
                    break;

                case Code.Ldloc:
                    LdNull();
                    break;

                case Ldloca:
                    LdNull();
                    break;

                case Ceq:
                    Pop();
                    break;

                case Add:
                    Pop();
                    break;

                case Starg_S:
                    Pop();
                    break;

                case Stloc_S:
                    Stloc(((VariableDefinition)il.Operand).Index);
                    break;

                case Dup:
                    evaluationStack.Push(evaluationStack.Peek());
                    break;

                case Sub:
                    Pop();
                    break;

                case Mul:
                    Pop();
                    break;

                case Div:
                    Pop();
                    break;

                case Div_Un:
                    Pop();
                    break;

                case Rem:
                    Pop();
                    break;

                case Rem_Un:
                    Pop();
                    break;

                case Newarr:
                    LdNull();
                    break;

                case Add_Ovf:
                    Pop();
                    break;

                case Add_Ovf_Un:
                    Pop();
                    break;

                case Mul_Ovf:
                    Pop();
                    break;

                case Mul_Ovf_Un:
                    Pop();
                    break;

                case Sub_Ovf:
                    Pop();
                    break;

                case Sub_Ovf_Un:
                    Pop();
                    break;

                case Starg:
                    Pop();
                    break;

                case Code.Stloc:
                    Pop();
                    break;

                case Ret:
                    if (IsCompletable(method.ReturnType) && evaluationStack.Peek() is Completable comp && !comp.Completed)
                        WriteIncompleteWarning();
                    break;

                default:
                    break;
            }
        }
    }
}