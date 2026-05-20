using CriWare;
using log4net.Core;
using System;
using System.Runtime.InteropServices;
using Unity.VisualScripting.YamlDotNet.Core.Tokens;
using UnityEngine;
using UnityEngine.UI;

namespace KMSynth
{
    public static class BitReinterp
    {
        // UInt32 のビットをそのまま float として解釈
        public static float UInt32BitsToSingle(UInt32 bits)
            => BitConverter.Int32BitsToSingle(unchecked((int)bits));

        // 逆変換：float のビットをそのまま UInt32 へ
        public static UInt32 SingleToUInt32Bits(float value)
            => unchecked((UInt32)BitConverter.SingleToInt32Bits(value));
    }
    public static class FMIDI
    {
        public static float createMidiMessage(byte b1)
        {
            return createMidiMessage(1, b1, 0, 0);
        }
        public static float createMidiMessage(byte b1, byte b2)
        {
            return createMidiMessage(2, b1, b2, 0);
        }
        public static float createMidiMessage(byte b1, byte b2, byte b3)
        {
            return createMidiMessage(3, b1, b2, b3);
        }

        private static float createMidiMessage(byte numByte, byte b1, byte b2, byte b3)
        {
            byte b0 = (byte)((1 << 7) | (0b0110 << 3) | (numByte & 0b111));
            var result_uint32 = (UInt32)((b0 << 24) | (b1 << 16) | (b2 << 8) | (b3 << 0));
            var result = BitReinterp.UInt32BitsToSingle(result_uint32);
            return result;
        }
    }

    public class KMSynthDllController : MonoBehaviour
    {


        private void OnEnable()
        {
        }
        private void OnDisable()
        {
        }

        public void TurnOnDebugSwitch()
        {
            CriAtomExAsr.SetEffectParameter("MasterOut", "Limiter", 0, 1.0f);
            //SendMidiMessage(0x90, 0x40, 0x64);
        }
        public void TurnOffDebugSwitch()
        {
            SendMidiMessage(0x80, 0x40, 0x64);
        }
        public void SetWaveFormToSin()
        {
            SendMidiMessage(0xB0, 0x02, 0x00);
            //CriAtomExAsr.SetEffectParameter("BUS1", "OnozukaPlan/KMSynth_CRI", 0xFFFF, msg);
        }
        public void SetWaveFormToSaw()
        {
            SendMidiMessage(0xB0, 0x02, 0x20);
            //CriAtomExAsr.SetEffectParameter("BUS1", "OnozukaPlan/KMSynth_CRI", 0x8000, 1);
        }
        public void SetWaveFormToPulse()
        {
            SendMidiMessage(0xB0, 0x02, 0x40);
            //CriAtomExAsr.SetEffectParameter("BUS1", "OnozukaPlan/KMSynth_CRI", 0x8000, 2);
        }
        public void SetWaveFormToTriangle()
        {
            SendMidiMessage(0xB0, 0x02, 0x60);
            //CriAtomExAsr.SetEffectParameter("BUS1", "OnozukaPlan/KMSynth_CRI", 0x8000, 3);
        }

        public void SendMidiMessage(byte b1, byte b2, byte b3)
        {
            // Send MIDI message
            var msg = FMIDI.createMidiMessage(b1, b2, b3);
            CriAtomExAsr.SetEffectParameter("BUS1", "OnozukaPlan/KMSynth_CRI", 0xFFFF, msg);

            // Log
            int bits = BitConverter.ToInt32(BitConverter.GetBytes(msg), 0);
            string bitString = Convert.ToString(bits, 2).PadLeft(32, '0');
            Debug.Log($"value: {msg},msg:0x{b1:X2} 0x{b2:X2} 0x{b3:X2} bits: {bitString}");
        }
    }
}