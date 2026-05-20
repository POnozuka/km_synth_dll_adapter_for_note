using System;
using System.Runtime.InteropServices;
using UnityEngine;
using CriWare;
using CriWare.Assets;

namespace KMSynth
{
    internal static class KMSynthDllLoaderNative
    {
        // Assets/Plugins/x86_64/PluginName.dllを参照する
        private const string PluginName = "KMSynth_CRI";

        [DllImport(PluginName, CallingConvention = CriWare.Common.pluginCallingConvention)]
        internal static extern IntPtr GetInterfaceWithVersion();
    }

    public class KMSynthDllLoader : MonoBehaviour
    {
        void Awake()
        {
            var itf = KMSynthDllLoaderNative.GetInterfaceWithVersion();

            if (itf != IntPtr.Zero)
            {
                var succeeded = CriAtomExAsr.RegisterEffectInterface(itf);
                if (succeeded)
                {
                    Debug.Log("KMSynth: GetInterfaceWithVersion() の登録に成功しました。");
                }
                else
                {
                    Debug.LogError("KMSynth: GetInterfaceWithVersion() の登録に失敗しました。");
                }
            }
            else
            {
                Debug.LogError("KMSynth: GetInterfaceWithVersion() が NULL を返しました。");
            }
        }
    }
}