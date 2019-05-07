using System;
using System.Collections.Generic;
using System.Text;
using System.Reflection;

namespace SocketProAdapter
{
    namespace ServerSide
    {
        public class CClientPeer : CSocketPeer
        {
            public delegate void DM_I0_R0();
            public delegate void DM_I0_R1<R0>(out R0 r0);
            public delegate void DM_I0_R2<R0, R1>(out R0 r0, out R1 r1);
            public delegate void DM_I0_R3<R0, R1, R2>(out R0 r0, out R1 r1, out R2 r2);
            public delegate void DM_I0_R4<R0, R1, R2, R3>(out R0 r0, out R1 r1, out R2 r2, out R3 r3);
            public delegate void DM_I0_R5<R0, R1, R2, R3, R4>(out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4);

            internal delegate R0 DRM_I0_R1<R0>();
            internal delegate R1 DRM_I0_R2<R0, R1>(out R0 r0);
            internal delegate R2 DRM_I0_R3<R0, R1, R2>(out R0 r0, out R1 r1);
            internal delegate R3 DRM_I0_R4<R0, R1, R2, R3>(out R0 r0, out R1 r1, out R2 r2);
            internal delegate R4 DRM_I0_R5<R0, R1, R2, R3, R4>(out R0 r0, out R1 r1, out R2 r2, out R3 r3);

            public delegate void DM_I1_R0<T0>(T0 t0);
            public delegate void DM_I1_R1<T0, R0>(T0 t0, out R0 r0);
            public delegate void DM_I1_R2<T0, R0, R1>(T0 t0, out R0 r0, out R1 r1);
            public delegate void DM_I1_R3<T0, R0, R1, R2>(T0 t0, out R0 r0, out R1 r1, out R2 r2);
            public delegate void DM_I1_R4<T0, R0, R1, R2, R3>(T0 t0, out R0 r0, out R1 r1, out R2 r2, out R3 r3);
            public delegate void DM_I1_R5<T0, R0, R1, R2, R3, R4>(T0 t0, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4);

            internal delegate R0 DRM_I1_R1<T0, R0>(T0 t0);
            internal delegate R1 DRM_I1_R2<T0, R0, R1>(T0 t0, out R0 r0);
            internal delegate R2 DRM_I1_R3<T0, R0, R1, R2>(T0 t0, out R0 r0, out R1 r1);
            internal delegate R3 DRM_I1_R4<T0, R0, R1, R2, R3>(T0 t0, out R0 r0, out R1 r1, out R2 r2);
            internal delegate R4 DRM_I1_R5<T0, R0, R1, R2, R3, R4>(T0 t0, out R0 r0, out R1 r1, out R2 r2, out R3 r3);


            public delegate void DM_I2_R0<T0, T1>(T0 t0, T1 t1);
            public delegate void DM_I2_R1<T0, T1, R0>(T0 t0, T1 t1, out R0 r0);
            public delegate void DM_I2_R2<T0, T1, R0, R1>(T0 t0, T1 t1, out R0 r0, out R1 r1);
            public delegate void DM_I2_R3<T0, T1, R0, R1, R2>(T0 t0, T1 t1, out R0 r0, out R1 r1, out R2 r2);
            public delegate void DM_I2_R4<T0, T1, R0, R1, R2, R3>(T0 t0, T1 t1, out R0 r0, out R1 r1, out R2 r2, out R3 r3);
            public delegate void DM_I2_R5<T0, T1, R0, R1, R2, R3, R4>(T0 t0, T1 t1, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4);

            internal delegate R0 DRM_I2_R1<T0, T1, R0>(T0 t0, T1 t1);
            internal delegate R1 DRM_I2_R2<T0, T1, R0, R1>(T0 t0, T1 t1, out R0 r0);
            internal delegate R2 DRM_I2_R3<T0, T1, R0, R1, R2>(T0 t0, T1 t1, out R0 r0, out R1 r1);
            internal delegate R3 DRM_I2_R4<T0, T1, R0, R1, R2, R3>(T0 t0, T1 t1, out R0 r0, out R1 r1, out R2 r2);
            internal delegate R4 DRM_I2_R5<T0, T1, R0, R1, R2, R3, R4>(T0 t0, T1 t1, out R0 r0, out R1 r1, out R2 r2, out R3 r3);

            public delegate void DM_I3_R0<T0, T1, T2>(T0 t0, T1 t1, T2 t2);
            public delegate void DM_I3_R1<T0, T1, T2, R0>(T0 t0, T1 t1, T2 t2, out R0 r0);
            public delegate void DM_I3_R2<T0, T1, T2, R0, R1>(T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1);
            public delegate void DM_I3_R3<T0, T1, T2, R0, R1, R2>(T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1, out R2 r2);
            public delegate void DM_I3_R4<T0, T1, T2, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1, out R2 r2, out R3 r3);
            public delegate void DM_I3_R5<T0, T1, T2, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4);

            internal delegate R0 DRM_I3_R1<T0, T1, T2, R0>(T0 t0, T1 t1, T2 t2);
            internal delegate R1 DRM_I3_R2<T0, T1, T2, R0, R1>(T0 t0, T1 t1, T2 t2, out R0 r0);
            internal delegate R2 DRM_I3_R3<T0, T1, T2, R0, R1, R2>(T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1);
            internal delegate R3 DRM_I3_R4<T0, T1, T2, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1, out R2 r2);
            internal delegate R4 DRM_I3_R5<T0, T1, T2, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1, out R2 r2, out R3 r3);

            public delegate void DM_I4_R0<T0, T1, T2, T3>(T0 t0, T1 t1, T2 t2, T3 t3);
            public delegate void DM_I4_R1<T0, T1, T2, T3, R0>(T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0);
            public delegate void DM_I4_R2<T0, T1, T2, T3, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1);
            public delegate void DM_I4_R3<T0, T1, T2, T3, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1, out R2 r2);
            public delegate void DM_I4_R4<T0, T1, T2, T3, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1, out R2 r2, out R3 r3);
            public delegate void DM_I4_R5<T0, T1, T2, T3, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4);

            internal delegate R0 DRM_I4_R1<T0, T1, T2, T3, R0>(T0 t0, T1 t1, T2 t2, T3 t3);
            internal delegate R1 DRM_I4_R2<T0, T1, T2, T3, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0);
            internal delegate R2 DRM_I4_R3<T0, T1, T2, T3, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1);
            internal delegate R3 DRM_I4_R4<T0, T1, T2, T3, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1, out R2 r2);
            internal delegate R4 DRM_I4_R5<T0, T1, T2, T3, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1, out R2 r2, out R3 r3);

            public delegate void DM_I5_R0<T0, T1, T2, T3, T4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4);
            public delegate void DM_I5_R1<T0, T1, T2, T3, T4, R0>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0);
            public delegate void DM_I5_R2<T0, T1, T2, T3, T4, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1);
            public delegate void DM_I5_R3<T0, T1, T2, T3, T4, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1, out R2 r2);
            public delegate void DM_I5_R4<T0, T1, T2, T3, T4, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1, out R2 r2, out R3 r3);
            public delegate void DM_I5_R5<T0, T1, T2, T3, T4, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4);

            internal delegate R0 DRM_I5_R1<T0, T1, T2, T3, T4, R0>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4);
            internal delegate R1 DRM_I5_R2<T0, T1, T2, T3, T4, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0);
            internal delegate R2 DRM_I5_R3<T0, T1, T2, T3, T4, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1);
            internal delegate R3 DRM_I5_R4<T0, T1, T2, T3, T4, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1, out R2 r2);
            internal delegate R4 DRM_I5_R5<T0, T1, T2, T3, T4, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1, out R2 r2, out R3 r3);

            public delegate void DM_I6_R0<T0, T1, T2, T3, T4, T5>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5);
            public delegate void DM_I6_R1<T0, T1, T2, T3, T4, T5, R0>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0);
            public delegate void DM_I6_R2<T0, T1, T2, T3, T4, T5, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1);
            public delegate void DM_I6_R3<T0, T1, T2, T3, T4, T5, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1, out R2 r2);
            public delegate void DM_I6_R4<T0, T1, T2, T3, T4, T5, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1, out R2 r2, out R3 r3);
            public delegate void DM_I6_R5<T0, T1, T2, T3, T4, T5, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4);

            internal delegate R0 DRM_I6_R1<T0, T1, T2, T3, T4, T5, R0>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5);
            internal delegate R1 DRM_I6_R2<T0, T1, T2, T3, T4, T5, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0);
            internal delegate R2 DRM_I6_R3<T0, T1, T2, T3, T4, T5, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1);
            internal delegate R3 DRM_I6_R4<T0, T1, T2, T3, T4, T5, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1, out R2 r2);
            internal delegate R4 DRM_I6_R5<T0, T1, T2, T3, T4, T5, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1, out R2 r2, out R3 r3);

            public delegate void DM_I7_R0<T0, T1, T2, T3, T4, T5, T6>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6);
            public delegate void DM_I7_R1<T0, T1, T2, T3, T4, T5, T6, R0>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0);
            public delegate void DM_I7_R2<T0, T1, T2, T3, T4, T5, T6, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1);
            public delegate void DM_I7_R3<T0, T1, T2, T3, T4, T5, T6, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1, out R2 r2);
            public delegate void DM_I7_R4<T0, T1, T2, T3, T4, T5, T6, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1, out R2 r2, out R3 r3);
            public delegate void DM_I7_R5<T0, T1, T2, T3, T4, T5, T6, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4);

            internal delegate R0 DRM_I7_R1<T0, T1, T2, T3, T4, T5, T6, R0>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6);
            internal delegate R1 DRM_I7_R2<T0, T1, T2, T3, T4, T5, T6, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0);
            internal delegate R2 DRM_I7_R3<T0, T1, T2, T3, T4, T5, T6, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1);
            internal delegate R3 DRM_I7_R4<T0, T1, T2, T3, T4, T5, T6, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1, out R2 r2);
            internal delegate R4 DRM_I7_R5<T0, T1, T2, T3, T4, T5, T6, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1, out R2 r2, out R3 r3);

            public delegate void DM_I8_R0<T0, T1, T2, T3, T4, T5, T6, T7>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7);
            public delegate void DM_I8_R1<T0, T1, T2, T3, T4, T5, T6, T7, R0>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0);
            public delegate void DM_I8_R2<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1);
            public delegate void DM_I8_R3<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1, out R2 r2);
            public delegate void DM_I8_R4<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1, out R2 r2, out R3 r3);
            public delegate void DM_I8_R5<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4);

            internal delegate R0 DRM_I8_R1<T0, T1, T2, T3, T4, T5, T6, T7, R0>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7);
            internal delegate R1 DRM_I8_R2<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0);
            internal delegate R2 DRM_I8_R3<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1);
            internal delegate R3 DRM_I8_R4<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1, out R2 r2);
            internal delegate R4 DRM_I8_R5<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1, out R2 r2, out R3 r3);

            public delegate void DM_I9_R0<T0, T1, T2, T3, T4, T5, T6, T7, T8>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8);
            public delegate void DM_I9_R1<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0);
            public delegate void DM_I9_R2<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1);
            public delegate void DM_I9_R3<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1, out R2 r2);
            public delegate void DM_I9_R4<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1, out R2 r2, out R3 r3);
            public delegate void DM_I9_R5<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4);

            internal delegate R0 DRM_I9_R1<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8);
            internal delegate R1 DRM_I9_R2<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0);
            internal delegate R2 DRM_I9_R3<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1);
            internal delegate R3 DRM_I9_R4<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1, out R2 r2);
            internal delegate R4 DRM_I9_R5<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1, out R2 r2, out R3 r3);

            public delegate void DM_I10_R0<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9);
            public delegate void DM_I10_R1<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0);
            public delegate void DM_I10_R2<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1);
            public delegate void DM_I10_R3<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1, out R2 r2);
            public delegate void DM_I10_R4<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1, out R2 r2, out R3 r3);
            public delegate void DM_I10_R5<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4);

            internal delegate R0 DRM_I10_R1<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9);
            internal delegate R1 DRM_I10_R2<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0);
            internal delegate R2 DRM_I10_R3<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1, R2>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1);
            internal delegate R3 DRM_I10_R4<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1, R2, R3>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1, out R2 r2);
            internal delegate R4 DRM_I10_R5<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1, R2, R3, R4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1, out R2 r2, out R3 r3);

            internal class CServerPushExImpl : IUPushEx
            {
                internal CSocketPeer m_sp;

                internal CServerPushExImpl(CSocketPeer sp)
                {
                    m_sp = sp;
                }

                #region IUPushEx Members

                public bool Publish(byte[] Message, uint[] Groups)
                {
                    uint size;
                    uint len;
                    if (Groups == null)
                        len = 0;
                    else
                        len = (uint)Groups.Length;
                    if (Message == null)
                        size = 0;
                    else
                        size = (uint)Message.Length;
                    unsafe
                    {
                        fixed (byte* buffer = Message)
                        {
                            fixed (uint* p = Groups)
                            {
                                return ServerCoreLoader.SpeakEx(m_sp.Handle, buffer, size, p, len);
                            }
                        }
                    }
                }

                public bool SendUserMessage(string UserId, byte[] Message)
                {
                    uint size;
                    if (Message == null)
                        size = 0;
                    else
                        size = (uint)Message.Length;
                    unsafe
                    {
                        fixed (byte* p = Message)
                        {
                            return ServerCoreLoader.SendUserMessageEx(m_sp.Handle, UserId, p, size);
                        }
                    }
                }

                #endregion

                #region IUPush Members

                public bool Publish(object Message, uint[] Groups)
                {
                    uint len;
                    if (Groups == null)
                        len = 0;
                    else
                        len = (uint)Groups.Length;
                    using (CScopeUQueue su = new CScopeUQueue())
                    {
                        CUQueue q = su.UQueue;
                        q.Save(Message);
                        unsafe
                        {
                            fixed (byte* buffer = q.m_bytes)
                            {
                                fixed (uint* p = Groups)
                                {
                                    return ServerCoreLoader.Speak(m_sp.Handle, buffer, q.GetSize(), p, len);
                                }
                            }
                        }
                    }
                }

                public bool Subscribe(uint[] Groups)
                {
                    uint len;
                    if (Groups == null)
                        len = 0;
                    else
                        len = (uint)Groups.Length;
                    unsafe
                    {
                        fixed (uint* p = Groups)
                        {
                            return ServerCoreLoader.Enter(m_sp.Handle, p, len);
                        }
                    }
                }

                public bool Unsubscribe()
                {
                    ServerCoreLoader.Exit(m_sp.Handle);
                    return true;
                }

                public bool SendUserMessage(object Message, string UserId)
                {
                    using (CScopeUQueue su = new CScopeUQueue())
                    {
                        CUQueue q = su.UQueue;
                        q.Save(Message);
                        unsafe
                        {
                            fixed (byte* p = q.m_bytes)
                            {
                                return ServerCoreLoader.SendUserMessage(m_sp.Handle, UserId, p, q.GetSize());
                            }
                        }
                    }
                }

                #endregion
            }

            internal CServerPushExImpl m_PushImpl;

            internal Dictionary<ushort, Delegate> m_dicDel = new Dictionary<ushort, Delegate>();

            private Delegate CreateDelgate(MethodInfo mi, uint inputs, uint outputs, Type[] gTypes)
            {
                Type delType = null;
                Type delGeneric = null;
                bool isVoid = (mi.ReturnType == typeof(void));
                switch (inputs)
                {
                    case 0:
                        switch (outputs)
                        {
                            case 0:
                                delType = typeof(DM_I0_R0);
                                break;
                            case 1:
                                if (isVoid)
                                    delType = typeof(DM_I0_R1<>);
                                else
                                    delType = typeof(DRM_I0_R1<>);
                                break;
                            case 2:
                                if (isVoid)
                                    delType = typeof(DM_I0_R2<,>);
                                else
                                    delType = typeof(DRM_I0_R2<,>);
                                break;
                            case 3:
                                if (isVoid)
                                    delType = typeof(DM_I0_R3<,,>);
                                else
                                    delType = typeof(DRM_I0_R3<,,>);
                                break;
                            case 4:
                                if (isVoid)
                                    delType = typeof(DM_I0_R4<,,,>);
                                else
                                    delType = typeof(DRM_I0_R4<,,,>);
                                break;
                            case 5:
                                if (isVoid)
                                    delType = typeof(DM_I0_R5<,,,,>);
                                else
                                    delType = typeof(DRM_I0_R5<,,,,>);
                                break;
                            default:
                                break;
                        }
                        break;
                    case 1:
                        switch (outputs)
                        {
                            case 0:
                                delType = typeof(DM_I1_R0<>);
                                break;
                            case 1:
                                if (isVoid)
                                    delType = typeof(DM_I1_R1<,>);
                                else
                                    delType = typeof(DRM_I1_R1<,>);
                                break;
                            case 2:
                                if (isVoid)
                                    delType = typeof(DM_I1_R2<,,>);
                                else
                                    delType = typeof(DRM_I1_R2<,,>);
                                break;
                            case 3:
                                if (isVoid)
                                    delType = typeof(DM_I1_R3<,,,>);
                                else
                                    delType = typeof(DRM_I1_R3<,,,>);
                                break;
                            case 4:
                                if (isVoid)
                                    delType = typeof(DM_I1_R4<,,,,>);
                                else
                                    delType = typeof(DRM_I1_R4<,,,,>);
                                break;
                            case 5:
                                if (isVoid)
                                    delType = typeof(DM_I1_R5<,,,,,>);
                                else
                                    delType = typeof(DRM_I1_R5<,,,,,>);
                                break;
                            default:
                                break;
                        }
                        break;
                    case 2:
                        switch (outputs)
                        {
                            case 0:
                                delType = typeof(DM_I2_R0<,>);
                                break;
                            case 1:
                                if (isVoid)
                                    delType = typeof(DM_I2_R1<,,>);
                                else
                                    delType = typeof(DRM_I2_R1<,,>);
                                break;
                            case 2:
                                if (isVoid)
                                    delType = typeof(DM_I2_R2<,,,>);
                                else
                                    delType = typeof(DRM_I2_R2<,,,>);
                                break;
                            case 3:
                                if (isVoid)
                                    delType = typeof(DM_I2_R3<,,,,>);
                                else
                                    delType = typeof(DRM_I2_R3<,,,,>);
                                break;
                            case 4:
                                if (isVoid)
                                    delType = typeof(DM_I2_R4<,,,,,>);
                                else
                                    delType = typeof(DRM_I2_R4<,,,,,>);
                                break;
                            case 5:
                                if (isVoid)
                                    delType = typeof(DM_I2_R5<,,,,,,>);
                                else
                                    delType = typeof(DRM_I2_R5<,,,,,,>);
                                break;
                            default:
                                break;
                        }
                        break;
                    case 3:
                        switch (outputs)
                        {
                            case 0:
                                delType = typeof(DM_I3_R0<,,>);
                                break;
                            case 1:
                                if (isVoid)
                                    delType = typeof(DM_I3_R1<,,,>);
                                else
                                    delType = typeof(DRM_I3_R1<,,,>);
                                break;
                            case 2:
                                if (isVoid)
                                    delType = typeof(DM_I3_R2<,,,,>);
                                else
                                    delType = typeof(DRM_I3_R2<,,,,>);
                                break;
                            case 3:
                                if (isVoid)
                                    delType = typeof(DM_I3_R3<,,,,,>);
                                else
                                    delType = typeof(DRM_I3_R3<,,,,,>);
                                break;
                            case 4:
                                if (isVoid)
                                    delType = typeof(DM_I3_R4<,,,,,,>);
                                else
                                    delType = typeof(DRM_I3_R4<,,,,,,>);
                                break;
                            case 5:
                                if (isVoid)
                                    delType = typeof(DM_I3_R5<,,,,,,,>);
                                else
                                    delType = typeof(DRM_I3_R5<,,,,,,,>);
                                break;
                            default:
                                break;
                        }
                        break;
                    case 4:
                        switch (outputs)
                        {
                            case 0:
                                delType = typeof(DM_I4_R0<,,,>);
                                break;
                            case 1:
                                if (isVoid)
                                    delType = typeof(DM_I4_R1<,,,,>);
                                else
                                    delType = typeof(DRM_I4_R1<,,,,>);
                                break;
                            case 2:
                                if (isVoid)
                                    delType = typeof(DM_I4_R2<,,,,,>);
                                else
                                    delType = typeof(DRM_I4_R2<,,,,,>);
                                break;
                            case 3:
                                if (isVoid)
                                    delType = typeof(DM_I4_R3<,,,,,,>);
                                else
                                    delType = typeof(DRM_I4_R3<,,,,,,>);
                                break;
                            case 4:
                                if (isVoid)
                                    delType = typeof(DM_I4_R4<,,,,,,,>);
                                else
                                    delType = typeof(DRM_I4_R4<,,,,,,,>);
                                break;
                            case 5:
                                if (isVoid)
                                    delType = typeof(DM_I4_R5<,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I4_R5<,,,,,,,,>);
                                break;
                            default:
                                break;
                        }
                        break;
                    case 5:
                        switch (outputs)
                        {
                            case 0:
                                delType = typeof(DM_I5_R0<,,,,>);
                                break;
                            case 1:
                                if (isVoid)
                                    delType = typeof(DM_I5_R1<,,,,,>);
                                else
                                    delType = typeof(DRM_I5_R1<,,,,,>);
                                break;
                            case 2:
                                if (isVoid)
                                    delType = typeof(DM_I5_R2<,,,,,,>);
                                else
                                    delType = typeof(DRM_I5_R2<,,,,,,>);
                                break;
                            case 3:
                                if (isVoid)
                                    delType = typeof(DM_I5_R3<,,,,,,,>);
                                else
                                    delType = typeof(DRM_I5_R3<,,,,,,,>);
                                break;
                            case 4:
                                if (isVoid)
                                    delType = typeof(DM_I5_R4<,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I5_R4<,,,,,,,,>);
                                break;
                            case 5:
                                if (isVoid)
                                    delType = typeof(DM_I5_R5<,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I5_R5<,,,,,,,,,>);
                                break;
                            default:
                                break;
                        }
                        break;
                    case 6:
                        switch (outputs)
                        {
                            case 0:
                                delType = typeof(DM_I6_R0<,,,,,>);
                                break;
                            case 1:
                                if (isVoid)
                                    delType = typeof(DM_I6_R1<,,,,,,>);
                                else
                                    delType = typeof(DRM_I6_R1<,,,,,,>);
                                break;
                            case 2:
                                if (isVoid)
                                    delType = typeof(DM_I6_R2<,,,,,,,>);
                                else
                                    delType = typeof(DRM_I6_R2<,,,,,,,>);
                                break;
                            case 3:
                                if (isVoid)
                                    delType = typeof(DM_I6_R3<,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I6_R3<,,,,,,,,>);
                                break;
                            case 4:
                                if (isVoid)
                                    delType = typeof(DM_I6_R4<,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I6_R4<,,,,,,,,,>);
                                break;
                            case 5:
                                if (isVoid)
                                    delType = typeof(DM_I6_R5<,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I6_R5<,,,,,,,,,,>);
                                break;
                            default:
                                break;
                        }
                        break;
                    case 7:
                        switch (outputs)
                        {
                            case 0:
                                delType = typeof(DM_I7_R0<,,,,,,>);
                                break;
                            case 1:
                                if (isVoid)
                                    delType = typeof(DM_I7_R1<,,,,,,,>);
                                else
                                    delType = typeof(DRM_I7_R1<,,,,,,,>);
                                break;
                            case 2:
                                if (isVoid)
                                    delType = typeof(DM_I7_R2<,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I7_R2<,,,,,,,,>);
                                break;
                            case 3:
                                if (isVoid)
                                    delType = typeof(DM_I7_R3<,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I7_R3<,,,,,,,,,>);
                                break;
                            case 4:
                                if (isVoid)
                                    delType = typeof(DM_I7_R4<,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I7_R4<,,,,,,,,,,>);
                                break;
                            case 5:
                                if (isVoid)
                                    delType = typeof(DM_I7_R5<,,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I7_R5<,,,,,,,,,,,>);
                                break;
                            default:
                                break;
                        }
                        break;
                    case 8:
                        switch (outputs)
                        {
                            case 0:
                                delType = typeof(DM_I8_R0<,,,,,,,>);
                                break;
                            case 1:
                                if (isVoid)
                                    delType = typeof(DM_I8_R1<,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I8_R1<,,,,,,,,>);
                                break;
                            case 2:
                                if (isVoid)
                                    delType = typeof(DM_I8_R2<,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I8_R2<,,,,,,,,,>);
                                break;
                            case 3:
                                if (isVoid)
                                    delType = typeof(DM_I8_R3<,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I8_R3<,,,,,,,,,,>);
                                break;
                            case 4:
                                if (isVoid)
                                    delType = typeof(DM_I8_R4<,,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I8_R4<,,,,,,,,,,,>);
                                break;
                            case 5:
                                if (isVoid)
                                    delType = typeof(DM_I8_R5<,,,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I8_R5<,,,,,,,,,,,,>);
                                break;
                            default:
                                break;
                        }
                        break;
                    case 9:
                        switch (outputs)
                        {
                            case 0:
                                delType = typeof(DM_I9_R0<,,,,,,,,>);
                                break;
                            case 1:
                                if (isVoid)
                                    delType = typeof(DM_I9_R1<,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I9_R1<,,,,,,,,,>);
                                break;
                            case 2:
                                if (isVoid)
                                    delType = typeof(DM_I9_R2<,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I9_R2<,,,,,,,,,,>);
                                break;
                            case 3:
                                if (isVoid)
                                    delType = typeof(DM_I9_R3<,,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I9_R3<,,,,,,,,,,,>);
                                break;
                            case 4:
                                if (isVoid)
                                    delType = typeof(DM_I9_R4<,,,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I9_R4<,,,,,,,,,,,,>);
                                break;
                            case 5:
                                if (isVoid)
                                    delType = typeof(DM_I9_R5<,,,,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I9_R5<,,,,,,,,,,,,,>);
                                break;
                            default:
                                break;
                        }
                        break;
                    case 10:
                        switch (outputs) {
                            case 0:
                                delType = typeof(DM_I10_R0<,,,,,,,,,>);
                                break;
                            case 1:
                                if (isVoid)
                                    delType = typeof(DM_I10_R1<,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I10_R1<,,,,,,,,,,>);
                                break;
                            case 2:
                                if (isVoid)
                                    delType = typeof(DM_I10_R2<,,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I10_R2<,,,,,,,,,,,>);
                                break;
                            case 3:
                                if (isVoid)
                                    delType = typeof(DM_I10_R3<,,,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I10_R3<,,,,,,,,,,,,>);
                                break;
                            case 4:
                                if (isVoid)
                                    delType = typeof(DM_I10_R4<,,,,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I10_R4<,,,,,,,,,,,,,>);
                                break;
                            case 5:
                                if (isVoid)
                                    delType = typeof(DM_I10_R5<,,,,,,,,,,,,,,>);
                                else
                                    delType = typeof(DRM_I10_R5<,,,,,,,,,,,,,,>);
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
                if (inputs + outputs > 0)
                    delGeneric = delType.MakeGenericType(gTypes);
                else
                    delGeneric = delType;
                return Delegate.CreateDelegate(delGeneric, this, mi);
            }

            private void SetDel(Type type)
            {
                MethodInfo[] mis = type.GetMethods(BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.GetField);
                foreach (MethodInfo mi in mis)
                {
                    bool notVoid = (mi.ReturnType != typeof(void));
                    RequestAttr[] ras = (RequestAttr[])mi.GetCustomAttributes(typeof(RequestAttr), true);
                    if (ras != null && ras.Length > 0)
                    {
                        uint input = 0;
                        uint output = 0;
                        int index = 0;
                        ParameterInfo[] pis = mi.GetParameters();
                        Type[] gTypes = new Type[pis.Length + (notVoid ? 1 : 0)];
                        foreach (ParameterInfo pi in pis)
                        {
                            if (pi.IsOut || pi.ParameterType.IsByRef)
                            {
                                ++output;
                                string typeName = pi.ParameterType.FullName.Substring(0, pi.ParameterType.FullName.IndexOf('&'));
                                gTypes[index] = Type.GetType(typeName);
                                if (gTypes[index] == null)
                                {
                                    foreach (var a in AppDomain.CurrentDomain.GetAssemblies())
                                    {
                                        Type t = a.GetType(typeName);
                                        if (t != null)
                                        {
                                            gTypes[index] = t;
                                            break;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                ++input;
                                gTypes[index] = pi.ParameterType;
                            }
                            ++index;
                        }

                        if (notVoid)
                        {
                            gTypes[input + output] = mi.ReturnType;
                            ++output;
                        }

                        m_dicDel[ras[0].RequestID] = CreateDelgate(mi, input, output, gTypes);
                    }
                }
            }

            private void SetDel()
            {
                Type type = GetType();
                while (type != null)
                {
                    SetDel(type);
                    type = type.BaseType;
                }
            }

            public CClientPeer()
            {
                m_PushImpl = new CServerPushExImpl(this);
                SetDel();
            }

            public IUPushEx Push
            {
                get
                {
                    return m_PushImpl;
                }
            }

            public bool StartBatching()
            {
                return ServerCoreLoader.StartBatching(Handle);
            }

            public bool CommitBatching()
            {
                return ServerCoreLoader.CommitBatching(Handle);
            }

            public bool AbortBatching()
            {
                return ServerCoreLoader.AbortBatching(Handle);
            }

            /// <summary>
            /// Dequeue messages from a persistent message queue
            /// </summary>
            /// <param name="qHandle">A handle representing a server persistent message queue</param>
            /// <param name="messageCount">An expected count of messages</param>
            /// <param name="bNotifiedWhenAvailable">A boolean value if this peer client will be notified once a message is available</param>
            /// <returns>A 8-byte long value. Its high-order 4-byte integer represents the actual bytes of dequeued messages; and its low-order 4-byte integer is the number of dequeued messages</returns>
            public ulong Dequeue(uint qHandle, uint messageCount, bool bNotifiedWhenAvailable)
            {
                return Dequeue(qHandle, messageCount, bNotifiedWhenAvailable, (uint)0);
            }

            /// <summary>
            /// Dequeue messages from a persistent message queue
            /// </summary>
            /// <param name="qHandle">A handle representing a server persistent message queue</param>
            /// <param name="messageCount">An expected count of messages</param>
            /// <param name="bNotifiedWhenAvailable">A boolean value if this peer client will be notified once a message is available</param>
            /// <param name="waitTime">A time-out value in ms for waiting for a message. It defaults to zero.</param>
            /// <returns>A 8-byte long value. Its high-order 4-byte integer represents the actual bytes of dequeued messages; and its low-order 4-byte integer is the number of dequeued messages</returns>
            public virtual ulong Dequeue(uint qHandle, uint messageCount, bool bNotifiedWhenAvailable, uint waitTime)
            {
                return ServerCoreLoader.Dequeue(qHandle, Handle, messageCount, bNotifiedWhenAvailable, waitTime);
            }

            /// <summary>
            ///  Dequeue messages from a persistent message queue
            /// </summary>
            /// <param name="qHandle">A handle representing a server persistent message queue</param>
            /// <param name="bNotifiedWhenAvailable">A boolean value if this peer client will be notified once a message is available</param>
            /// <returns>A 8-byte long value. Its high-order 4-byte integer represents the actual bytes of dequeued messages; and its low-order 4-byte integer is the number of dequeued messages</returns>
            public ulong Dequeue(uint qHandle, bool bNotifiedWhenAvailable)
            {
                return Dequeue(qHandle, bNotifiedWhenAvailable, 8 * 1024, (uint)0);
            }

            /// <summary>
            ///  Dequeue messages from a persistent message queue
            /// </summary>
            /// <param name="qHandle">A handle representing a server persistent message queue</param>
            /// <param name="bNotifiedWhenAvailable">A boolean value if this peer client will be notified once a message is available</param>
            /// <param name="maxBytes">The max number of message bytes. It defaults to 8 kilobytes</param>
            /// <returns>A 8-byte long value. Its high-order 4-byte integer represents the actual bytes of dequeued messages; and its low-order 4-byte integer is the number of dequeued messages</returns>
            public ulong Dequeue(uint qHandle, bool bNotifiedWhenAvailable, uint maxBytes)
            {
                return Dequeue(qHandle, bNotifiedWhenAvailable, maxBytes, (uint)0);
            }

            /// <summary>
            /// Dequeue messages from a persistent message queue
            /// </summary>
            /// <param name="qHandle">A handle representing a server persistent message queue</param>
            /// <param name="bNotifiedWhenAvailable">A boolean value if this peer client will be notified once a message is available</param>
            /// <param name="maxBytes">The max number of message bytes. It defaults to 8 kilobytes</param>
            /// <param name="waitTime">A time-out value in ms for waiting for a message. It defaults to zero</param>
            /// <returns>A 8-byte long value. Its high-order 4-byte integer represents the actual bytes of dequeued messages; and its low-order 4-byte integer is the number of dequeued messages</returns>
            public virtual ulong Dequeue(uint qHandle, bool bNotifiedWhenAvailable, uint maxBytes, uint waitTime)
            {
                return ServerCoreLoader.Dequeue2(qHandle, Handle, maxBytes, bNotifiedWhenAvailable, waitTime);
            }

            /// <summary>
            /// Enable or disable client side dequeue from server side
            /// </summary>
            /// <param name="enable">True for enabling client side dequeue; and false for disabling client side dequeue</param>
            public void EnableClientDequeue(bool enable)
            {
                ServerCoreLoader.EnableClientDequeue(Handle, enable);
            }

            public tagZipLevel ZipLevel
            {
                get
                {
                    return ServerCoreLoader.GetZipLevel(Handle);
                }
                set
                {
                    ServerCoreLoader.SetZipLevel(Handle, value);
                }
            }

            public bool Zip
            {
                get
                {
                    return ServerCoreLoader.GetZip(Handle);
                }
                set
                {
                    ServerCoreLoader.SetZip(Handle, value);
                }
            }

            public bool DequeuedMessageAborted
            {
                get
                {
                    return ServerCoreLoader.IsDequeuedMessageAborted(Handle);
                }
            }

            public void AbortDequeuedMessage()
            {
                ServerCoreLoader.AbortDequeuedMessage(Handle);
            }

            public bool IsDequeueRequest
            {
                get
                {
                    return ServerCoreLoader.IsDequeueRequest(Handle);
                }
            }

            public tagOperationSystem GetPeerOs(ref bool bigEndian)
            {
                return ServerCoreLoader.GetPeerOs(Handle, ref bigEndian);
            }

            public tagOperationSystem GetPeerOs()
            {
                bool endian = false;
                return ServerCoreLoader.GetPeerOs(Handle, ref endian);
            }

            public uint BytesBatched
            {
                get
                {
                    return ServerCoreLoader.GetBytesBatched(Handle);
                }
            }

            protected virtual void OnPublishEx(uint[] groups, byte[] message)
            {

            }

            protected virtual void OnSendUserMessageEx(string receiver, byte[] message)
            {

            }

            protected virtual void OnFastRequestArrive(ushort requestId, uint len)
            {

            }

            internal void OnFast(ushort requestId, uint len)
            {
                OnFastRequestArrive(requestId, len);
            }

            protected virtual int OnSlowRequestArrive(ushort requestId, uint len)
            {
                return 0;
            }

            internal int OnSlow(ushort requestId, uint len)
            {
                return OnSlowRequestArrive(requestId, len);
            }

            protected uint M_I0_R0(DM_I0_R0 f)
            {
                f();
                return SendResult(CurrentRequestID);
            }

            protected uint M_I0_R1<R0>(DM_I0_R1<R0> f)
            {
                R0 r0;
                f(out r0);
                return SendResult(CurrentRequestID, r0);
            }

            protected uint M_I0_R2<R0, R1>(DM_I0_R2<R0, R1> f)
            {
                R0 r0;
                R1 r1;
                f(out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1);
            }

            protected uint M_I0_R3<R0, R1, R2>(DM_I0_R3<R0, R1, R2> f)
            {
                R0 r0;
                R1 r1;
                R2 r2;
                f(out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            protected uint M_I0_R4<R0, R1, R2, R3>(DM_I0_R4<R0, R1, R2, R3> f)
            {
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                f(out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            protected uint M_I0_R5<R0, R1, R2, R3, R4>(DM_I0_R5<R0, R1, R2, R3, R4> f)
            {
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4;
                f(out r0, out r1, out r2, out r3, out r4);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            internal uint RM_I0_R1<R0>(DRM_I0_R1<R0> f)
            {
                R0 r0 = f();
                return SendResult(CurrentRequestID, r0);
            }

            internal uint RM_I0_R2<R0, R1>(DRM_I0_R2<R0, R1> f)
            {
                R0 r0;
                R1 r1 = f(out r0);
                return SendResult(CurrentRequestID, r0, r1);
            }

            internal uint RM_I0_R3<R0, R1, R2>(DRM_I0_R3<R0, R1, R2> f)
            {
                R0 r0;
                R1 r1;
                R2 r2 = f(out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            internal uint RM_I0_R4<R0, R1, R2, R3>(DRM_I0_R4<R0, R1, R2, R3> f)
            {
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3 = f(out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            internal uint RM_I0_R5<R0, R1, R2, R3, R4>(DRM_I0_R5<R0, R1, R2, R3, R4> f)
            {
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4 = f(out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            protected uint M_I1_R0<A0>(DM_I1_R0<A0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                f(a0);
                return SendResult(CurrentRequestID);
            }

            protected uint M_I1_R1<A0, R0>(DM_I1_R1<A0, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                R0 r0 = default(R0);
                f(a0, out r0);
                return SendResult(CurrentRequestID, r0);
            }

            protected uint M_I1_R2<A0, R0, R1>(DM_I1_R2<A0, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                R0 r0;
                R1 r1;
                f(a0, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1);
            }

            protected uint M_I1_R3<A0, R0, R1, R2>(DM_I1_R3<A0, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                R0 r0;
                R1 r1;
                R2 r2;
                f(a0, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            protected uint M_I1_R4<A0, R0, R1, R2, R3>(DM_I1_R4<A0, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                f(a0, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            protected uint M_I1_R5<A0, R0, R1, R2, R3, R4>(DM_I1_R5<A0, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4;
                f(a0, out r0, out r1, out r2, out r3, out r4);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            internal uint RM_I1_R1<A0, R0>(DRM_I1_R1<A0, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                R0 r0 = f(a0);
                return SendResult(CurrentRequestID, r0);
            }

            internal uint RM_I1_R2<A0, R0, R1>(DRM_I1_R2<A0, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                R0 r0;
                R1 r1 = f(a0, out r0);
                return SendResult(CurrentRequestID, r0, r1);
            }

            internal uint RM_I1_R3<A0, R0, R1, R2>(DRM_I1_R3<A0, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                R0 r0;
                R1 r1;
                R2 r2 = f(a0, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            internal uint RM_I1_R4<A0, R0, R1, R2, R3>(DRM_I1_R4<A0, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3 = f(a0, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            internal uint RM_I1_R5<A0, R0, R1, R2, R3, R4>(DRM_I1_R5<A0, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4 = f(a0, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            protected uint M_I2_R0<A0, A1>(DM_I2_R0<A0, A1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                f(a0, a1);
                return SendResult(CurrentRequestID);
            }

            protected uint M_I2_R1<A0, A1, R0>(DM_I2_R1<A0, A1, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                R0 r0;
                f(a0, a1, out r0);
                return SendResult(CurrentRequestID, r0);
            }

            protected uint M_I2_R2<A0, A1, R0, R1>(DM_I2_R2<A0, A1, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                R0 r0;
                R1 r1;
                f(a0, a1, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1);
            }

            protected uint M_I2_R3<A0, A1, R0, R1, R2>(DM_I2_R3<A0, A1, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                R0 r0;
                R1 r1;
                R2 r2;
                f(a0, a1, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            protected uint M_I2_R4<A0, A1, R0, R1, R2, R3>(DM_I2_R4<A0, A1, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                f(a0, a1, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            protected uint M_I2_R5<A0, A1, R0, R1, R2, R3, R4>(DM_I2_R5<A0, A1, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4;
                f(a0, a1, out r0, out r1, out r2, out r3, out r4);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            internal uint RM_I2_R1<A0, A1, R0>(DRM_I2_R1<A0, A1, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                R0 r0 = f(a0, a1);
                return SendResult(CurrentRequestID, r0);
            }

            internal uint RM_I2_R2<A0, A1, R0, R1>(DRM_I2_R2<A0, A1, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                R0 r0;
                R1 r1 = f(a0, a1, out r0);
                return SendResult(CurrentRequestID, r0, r1);
            }

            internal uint RM_I2_R3<A0, A1, R0, R1, R2>(DRM_I2_R3<A0, A1, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                R0 r0;
                R1 r1;
                R2 r2 = f(a0, a1, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            internal uint RM_I2_R4<A0, A1, R0, R1, R2, R3>(DRM_I2_R4<A0, A1, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3 = f(a0, a1, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            internal uint RM_I2_R5<A0, A1, R0, R1, R2, R3, R4>(DRM_I2_R5<A0, A1, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4 = f(a0, a1, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            protected uint M_I3_R0<A0, A1, A2>(DM_I3_R0<A0, A1, A2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                f(a0, a1, a2);
                return SendResult(CurrentRequestID);
            }

            protected uint M_I3_R1<A0, A1, A2, R0>(DM_I3_R1<A0, A1, A2, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                R0 r0;
                f(a0, a1, a2, out r0);
                return SendResult(CurrentRequestID, r0);
            }

            protected uint M_I3_R2<A0, A1, A2, R0, R1>(DM_I3_R2<A0, A1, A2, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                R0 r0;
                R1 r1;
                f(a0, a1, a2, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1);
            }

            protected uint M_I3_R3<A0, A1, A2, R0, R1, R2>(DM_I3_R3<A0, A1, A2, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                R0 r0;
                R1 r1;
                R2 r2;
                f(a0, a1, a2, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            protected uint M_I3_R4<A0, A1, A2, R0, R1, R2, R3>(DM_I3_R4<A0, A1, A2, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                f(a0, a1, a2, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            protected uint M_I3_R5<A0, A1, A2, R0, R1, R2, R3, R4>(DM_I3_R5<A0, A1, A2, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4;
                f(a0, a1, a2, out r0, out r1, out r2, out r3, out r4);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            internal uint RM_I3_R1<A0, A1, A2, R0>(DRM_I3_R1<A0, A1, A2, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                R0 r0 = f(a0, a1, a2);
                return SendResult(CurrentRequestID, r0);
            }

            internal uint RM_I3_R2<A0, A1, A2, R0, R1>(DRM_I3_R2<A0, A1, A2, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                R0 r0;
                R1 r1 = f(a0, a1, a2, out r0);
                return SendResult(CurrentRequestID, r0, r1);
            }

            internal uint RM_I3_R3<A0, A1, A2, R0, R1, R2>(DRM_I3_R3<A0, A1, A2, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                R0 r0;
                R1 r1;
                R2 r2 = f(a0, a1, a2, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            internal uint RM_I3_R4<A0, A1, A2, R0, R1, R2, R3>(DRM_I3_R4<A0, A1, A2, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3 = f(a0, a1, a2, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            internal uint RM_I3_R5<A0, A1, A2, R0, R1, R2, R3, R4>(DRM_I3_R5<A0, A1, A2, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4 = f(a0, a1, a2, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            protected uint M_I4_R0<A0, A1, A2, A3>(DM_I4_R0<A0, A1, A2, A3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                f(a0, a1, a2, a3);
                return SendResult(CurrentRequestID);
            }

            protected uint M_I4_R1<A0, A1, A2, A3, R0>(DM_I4_R1<A0, A1, A2, A3, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                R0 r0;
                f(a0, a1, a2, a3, out r0);
                return SendResult(CurrentRequestID, r0);
            }

            protected uint M_I4_R2<A0, A1, A2, A3, R0, R1>(DM_I4_R2<A0, A1, A2, A3, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                R0 r0;
                R1 r1;
                f(a0, a1, a2, a3, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1);
            }

            protected uint M_I4_R3<A0, A1, A2, A3, R0, R1, R2>(DM_I4_R3<A0, A1, A2, A3, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                R0 r0;
                R1 r1;
                R2 r2;
                f(a0, a1, a2, a3, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            protected uint M_I4_R4<A0, A1, A2, A3, R0, R1, R2, R3>(DM_I4_R4<A0, A1, A2, A3, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                f(a0, a1, a2, a3, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            protected uint M_I4_R5<A0, A1, A2, A3, R0, R1, R2, R3, R4>(DM_I4_R5<A0, A1, A2, A3, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4;
                f(a0, a1, a2, a3, out r0, out r1, out r2, out r3, out r4);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            internal uint RM_I4_R1<A0, A1, A2, A3, R0>(DRM_I4_R1<A0, A1, A2, A3, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                R0 r0 = f(a0, a1, a2, a3);
                return SendResult(CurrentRequestID, r0);
            }

            internal uint RM_I4_R2<A0, A1, A2, A3, R0, R1>(DRM_I4_R2<A0, A1, A2, A3, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                R0 r0;
                R1 r1 = f(a0, a1, a2, a3, out r0);
                return SendResult(CurrentRequestID, r0, r1);
            }

            internal uint RM_I4_R3<A0, A1, A2, A3, R0, R1, R2>(DRM_I4_R3<A0, A1, A2, A3, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                R0 r0;
                R1 r1;
                R2 r2 = f(a0, a1, a2, a3, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            internal uint RM_I4_R4<A0, A1, A2, A3, R0, R1, R2, R3>(DRM_I4_R4<A0, A1, A2, A3, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3 = f(a0, a1, a2, a3, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            internal uint RM_I4_R5<A0, A1, A2, A3, R0, R1, R2, R3, R4>(DRM_I4_R5<A0, A1, A2, A3, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4 = f(a0, a1, a2, a3, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            protected uint M_I5_R0<A0, A1, A2, A3, A4>(DM_I5_R0<A0, A1, A2, A3, A4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                f(a0, a1, a2, a3, a4);
                return SendResult(CurrentRequestID);
            }

            protected uint M_I5_R1<A0, A1, A2, A3, A4, R0>(DM_I5_R1<A0, A1, A2, A3, A4, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                R0 r0;
                f(a0, a1, a2, a3, a4, out r0);
                return SendResult(CurrentRequestID, r0);
            }

            protected uint M_I5_R2<A0, A1, A2, A3, A4, R0, R1>(DM_I5_R2<A0, A1, A2, A3, A4, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                R0 r0;
                R1 r1;
                f(a0, a1, a2, a3, a4, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1);
            }

            protected uint M_I5_R3<A0, A1, A2, A3, A4, R0, R1, R2>(DM_I5_R3<A0, A1, A2, A3, A4, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                R0 r0;
                R1 r1;
                R2 r2;
                f(a0, a1, a2, a3, a4, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            protected uint M_I5_R4<A0, A1, A2, A3, A4, R0, R1, R2, R3>(DM_I5_R4<A0, A1, A2, A3, A4, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                f(a0, a1, a2, a3, a4, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            protected uint M_I5_R5<A0, A1, A2, A3, A4, R0, R1, R2, R3, R4>(DM_I5_R5<A0, A1, A2, A3, A4, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4;
                f(a0, a1, a2, a3, a4, out r0, out r1, out r2, out r3, out r4);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            internal uint RM_I5_R1<A0, A1, A2, A3, A4, R0>(DRM_I5_R1<A0, A1, A2, A3, A4, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                R0 r0 = f(a0, a1, a2, a3, a4);
                return SendResult(CurrentRequestID, r0);
            }

            internal uint RM_I5_R2<A0, A1, A2, A3, A4, R0, R1>(DRM_I5_R2<A0, A1, A2, A3, A4, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                R0 r0;
                R1 r1 = f(a0, a1, a2, a3, a4, out r0);
                return SendResult(CurrentRequestID, r0, r1);
            }

            internal uint RM_I5_R3<A0, A1, A2, A3, A4, R0, R1, R2>(DRM_I5_R3<A0, A1, A2, A3, A4, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                R0 r0;
                R1 r1;
                R2 r2 = f(a0, a1, a2, a3, a4, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            internal uint RM_I5_R4<A0, A1, A2, A3, A4, R0, R1, R2, R3>(DRM_I5_R4<A0, A1, A2, A3, A4, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3 = f(a0, a1, a2, a3, a4, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            internal uint RM_I5_R5<A0, A1, A2, A3, A4, R0, R1, R2, R3, R4>(DRM_I5_R5<A0, A1, A2, A3, A4, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4 = f(a0, a1, a2, a3, a4, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            protected uint M_I6_R0<A0, A1, A2, A3, A4, A5>(DM_I6_R0<A0, A1, A2, A3, A4, A5> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                f(a0, a1, a2, a3, a4, a5);
                return SendResult(CurrentRequestID);
            }

            protected uint M_I6_R1<A0, A1, A2, A3, A4, A5, R0>(DM_I6_R1<A0, A1, A2, A3, A4, A5, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                R0 r0;
                f(a0, a1, a2, a3, a4, a5, out r0);
                return SendResult(CurrentRequestID, r0);
            }

            protected uint M_I6_R2<A0, A1, A2, A3, A4, A5, R0, R1>(DM_I6_R2<A0, A1, A2, A3, A4, A5, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                R0 r0;
                R1 r1;
                f(a0, a1, a2, a3, a4, a5, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1);
            }

            protected uint M_I6_R3<A0, A1, A2, A3, A4, A5, R0, R1, R2>(DM_I6_R3<A0, A1, A2, A3, A4, A5, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                R0 r0;
                R1 r1;
                R2 r2;
                f(a0, a1, a2, a3, a4, a5, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            protected uint M_I6_R4<A0, A1, A2, A3, A4, A5, R0, R1, R2, R3>(DM_I6_R4<A0, A1, A2, A3, A4, A5, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                f(a0, a1, a2, a3, a4, a5, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            protected uint M_I6_R5<A0, A1, A2, A3, A4, A5, R0, R1, R2, R3, R4>(DM_I6_R5<A0, A1, A2, A3, A4, A5, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4;
                f(a0, a1, a2, a3, a4, a5, out r0, out r1, out r2, out r3, out r4);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            internal uint RM_I6_R1<A0, A1, A2, A3, A4, A5, R0>(DRM_I6_R1<A0, A1, A2, A3, A4, A5, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                R0 r0 = f(a0, a1, a2, a3, a4, a5);
                return SendResult(CurrentRequestID, r0);
            }

            internal uint RM_I6_R2<A0, A1, A2, A3, A4, A5, R0, R1>(DRM_I6_R2<A0, A1, A2, A3, A4, A5, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                R0 r0;
                R1 r1 = f(a0, a1, a2, a3, a4, a5, out r0);
                return SendResult(CurrentRequestID, r0, r1);
            }

            internal uint RM_I6_R3<A0, A1, A2, A3, A4, A5, R0, R1, R2>(DRM_I6_R3<A0, A1, A2, A3, A4, A5, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                R0 r0;
                R1 r1;
                R2 r2 = f(a0, a1, a2, a3, a4, a5, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            internal uint RM_I6_R4<A0, A1, A2, A3, A4, A5, R0, R1, R2, R3>(DRM_I6_R4<A0, A1, A2, A3, A4, A5, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3 = f(a0, a1, a2, a3, a4, a5, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            internal uint RM_I6_R5<A0, A1, A2, A3, A4, A5, R0, R1, R2, R3, R4>(DRM_I6_R5<A0, A1, A2, A3, A4, A5, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4 = f(a0, a1, a2, a3, a4, a5, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            protected uint M_I7_R0<A0, A1, A2, A3, A4, A5, A6>(DM_I7_R0<A0, A1, A2, A3, A4, A5, A6> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                f(a0, a1, a2, a3, a4, a5, a6);
                return SendResult(CurrentRequestID);
            }

            protected uint M_I7_R1<A0, A1, A2, A3, A4, A5, A6, R0>(DM_I7_R1<A0, A1, A2, A3, A4, A5, A6, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                R0 r0;
                f(a0, a1, a2, a3, a4, a5, a6, out r0);
                return SendResult(CurrentRequestID, r0);
            }

            protected uint M_I7_R2<A0, A1, A2, A3, A4, A5, A6, R0, R1>(DM_I7_R2<A0, A1, A2, A3, A4, A5, A6, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                R0 r0;
                R1 r1;
                f(a0, a1, a2, a3, a4, a5, a6, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1);
            }

            protected uint M_I7_R3<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2>(DM_I7_R3<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                R0 r0;
                R1 r1;
                R2 r2;
                f(a0, a1, a2, a3, a4, a5, a6, out r0, out r1, out  r2);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            protected uint M_I7_R4<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3>(DM_I7_R4<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                f(a0, a1, a2, a3, a4, a5, a6, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            protected uint M_I7_R5<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3, R4>(DM_I7_R5<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4;
                f(a0, a1, a2, a3, a4, a5, a6, out r0, out r1, out r2, out r3, out r4);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            internal uint RM_I7_R1<A0, A1, A2, A3, A4, A5, A6, R0>(DRM_I7_R1<A0, A1, A2, A3, A4, A5, A6, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                R0 r0 = f(a0, a1, a2, a3, a4, a5, a6);
                return SendResult(CurrentRequestID, r0);
            }

            internal uint RM_I7_R2<A0, A1, A2, A3, A4, A5, A6, R0, R1>(DRM_I7_R2<A0, A1, A2, A3, A4, A5, A6, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                R0 r0;
                R1 r1 = f(a0, a1, a2, a3, a4, a5, a6, out r0);
                return SendResult(CurrentRequestID, r0, r1);
            }

            internal uint RM_I7_R3<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2>(DRM_I7_R3<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                R0 r0;
                R1 r1;
                R2 r2 = f(a0, a1, a2, a3, a4, a5, a6, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            internal uint RM_I7_R4<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3>(DRM_I7_R4<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3 = f(a0, a1, a2, a3, a4, a5, a6, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            internal uint RM_I7_R5<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3, R4>(DRM_I7_R5<A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4 = f(a0, a1, a2, a3, a4, a5, a6, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            protected uint M_I8_R0<A0, A1, A2, A3, A4, A5, A6, A7>(DM_I8_R0<A0, A1, A2, A3, A4, A5, A6, A7> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                f(a0, a1, a2, a3, a4, a5, a6, a7);
                return SendResult(CurrentRequestID);
            }

            protected uint M_I8_R1<A0, A1, A2, A3, A4, A5, A6, A7, R0>(DM_I8_R1<A0, A1, A2, A3, A4, A5, A6, A7, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                R0 r0;
                f(a0, a1, a2, a3, a4, a5, a6, a7, out  r0);
                return SendResult(CurrentRequestID, r0);
            }

            protected uint M_I8_R2<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1>(DM_I8_R2<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                R0 r0;
                R1 r1;
                f(a0, a1, a2, a3, a4, a5, a6, a7, out r0, out  r1);
                return SendResult(CurrentRequestID, r0, r1);
            }

            protected uint M_I8_R3<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2>(DM_I8_R3<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                R0 r0;
                R1 r1;
                R2 r2;
                f(a0, a1, a2, a3, a4, a5, a6, a7, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            protected uint M_I8_R4<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3>(DM_I8_R4<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                f(a0, a1, a2, a3, a4, a5, a6, a7, out r0, out r1, out r2, out  r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            protected uint M_I8_R5<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3, R4>(DM_I8_R5<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4;
                f(a0, a1, a2, a3, a4, a5, a6, a7, out r0, out r1, out r2, out r3, out  r4);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            internal uint RM_I8_R1<A0, A1, A2, A3, A4, A5, A6, A7, R0>(DRM_I8_R1<A0, A1, A2, A3, A4, A5, A6, A7, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                R0 r0 = f(a0, a1, a2, a3, a4, a5, a6, a7);
                return SendResult(CurrentRequestID, r0);
            }

            internal uint RM_I8_R2<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1>(DRM_I8_R2<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                R0 r0;
                R1 r1 = f(a0, a1, a2, a3, a4, a5, a6, a7, out r0);
                return SendResult(CurrentRequestID, r0, r1);
            }

            internal uint RM_I8_R3<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2>(DRM_I8_R3<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                R0 r0;
                R1 r1;
                R2 r2 = f(a0, a1, a2, a3, a4, a5, a6, a7, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            internal uint RM_I8_R4<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3>(DRM_I8_R4<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3 = f(a0, a1, a2, a3, a4, a5, a6, a7, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            internal uint RM_I8_R5<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3, R4>(DRM_I8_R5<A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4 = f(a0, a1, a2, a3, a4, a5, a6, a7, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            protected uint M_I9_R0<A0, A1, A2, A3, A4, A5, A6, A7, A8>(DM_I9_R0<A0, A1, A2, A3, A4, A5, A6, A7, A8> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                f(a0, a1, a2, a3, a4, a5, a6, a7, a8);
                return SendResult(CurrentRequestID);
            }

            protected uint M_I9_R1<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0>(DM_I9_R1<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                R0 r0;
                f(a0, a1, a2, a3, a4, a5, a6, a7, a8, out r0);
                return SendResult(CurrentRequestID, r0);
            }

            protected uint M_I9_R2<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1>(DM_I9_R2<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                R0 r0;
                R1 r1;
                f(a0, a1, a2, a3, a4, a5, a6, a7, a8, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1);
            }

            protected uint M_I9_R3<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2>(DM_I9_R3<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                R0 r0;
                R1 r1;
                R2 r2;
                f(a0, a1, a2, a3, a4, a5, a6, a7, a8, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            protected uint M_I9_R4<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3>(DM_I9_R4<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                f(a0, a1, a2, a3, a4, a5, a6, a7, a8, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            protected uint M_I9_R5<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3, R4>(DM_I9_R5<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4;
                f(a0, a1, a2, a3, a4, a5, a6, a7, a8, out r0, out r1, out r2, out r3, out r4);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            internal uint RM_I9_R1<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0>(DRM_I9_R1<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                R0 r0 = f(a0, a1, a2, a3, a4, a5, a6, a7, a8);
                return SendResult(CurrentRequestID, r0);
            }

            internal uint RM_I9_R2<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1>(DRM_I9_R2<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                R0 r0;
                R1 r1 = f(a0, a1, a2, a3, a4, a5, a6, a7, a8, out r0);
                return SendResult(CurrentRequestID, r0, r1);
            }

            internal uint RM_I9_R3<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2>(DRM_I9_R3<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                R0 r0;
                R1 r1;
                R2 r2 = f(a0, a1, a2, a3, a4, a5, a6, a7, a8, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            internal uint RM_I9_R4<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3>(DRM_I9_R4<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3 = f(a0, a1, a2, a3, a4, a5, a6, a7, a8, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            internal uint RM_I9_R5<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3, R4>(DRM_I9_R5<A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4 = f(a0, a1, a2, a3, a4, a5, a6, a7, a8, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            protected uint M_I10_R0<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>(DM_I10_R0<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                A9 a9;
                UQueue.Load(out a9);
                f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
                return SendResult(CurrentRequestID);
            }

            protected uint M_I10_R1<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0>(DM_I10_R1<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                A9 a9;
                UQueue.Load(out a9);
                R0 r0;
                f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, out r0);
                return SendResult(CurrentRequestID, r0);
            }

            protected uint M_I10_R2<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1>(DM_I10_R2<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                A9 a9;
                UQueue.Load(out a9);
                R0 r0;
                R1 r1;
                f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1);
            }

            protected uint M_I10_R3<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2>(DM_I10_R3<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                A9 a9;
                UQueue.Load(out a9);
                R0 r0;
                R1 r1;
                R2 r2;
                f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            protected uint M_I10_R4<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3>(DM_I10_R4<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                A9 a9;
                UQueue.Load(out a9);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            protected uint M_I10_R5<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3, R4>(DM_I10_R5<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                A9 a9;
                UQueue.Load(out a9);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4;
                f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, out r0, out r1, out r2, out r3, out r4);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            internal uint RM_I10_R1<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0>(DRM_I10_R1<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                A9 a9;
                UQueue.Load(out a9);
                R0 r0 = f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
                return SendResult(CurrentRequestID, r0);
            }

            internal uint RM_I10_R2<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1>(DRM_I10_R2<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                A9 a9;
                UQueue.Load(out a9);
                R0 r0;
                R1 r1 = f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, out r0);
                return SendResult(CurrentRequestID, r0, r1);
            }

            internal uint RM_I10_R3<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2>(DRM_I10_R3<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                A9 a9;
                UQueue.Load(out a9);
                R0 r0;
                R1 r1;
                R2 r2 = f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, out r0, out r1);
                return SendResult(CurrentRequestID, r0, r1, r2);
            }

            internal uint RM_I10_R4<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3>(DRM_I10_R4<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                A9 a9;
                UQueue.Load(out a9);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3 = f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, out r0, out r1, out r2);
                return SendResult(CurrentRequestID, r0, r1, r2, r3);
            }

            internal uint RM_I10_R5<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3, R4>(DRM_I10_R5<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3, R4> f)
            {
                A0 a0;
                UQueue.Load(out a0);
                A1 a1;
                UQueue.Load(out a1);
                A2 a2;
                UQueue.Load(out a2);
                A3 a3;
                UQueue.Load(out a3);
                A4 a4;
                UQueue.Load(out a4);
                A5 a5;
                UQueue.Load(out a5);
                A6 a6;
                UQueue.Load(out a6);
                A7 a7;
                UQueue.Load(out a7);
                A8 a8;
                UQueue.Load(out a8);
                A9 a9;
                UQueue.Load(out a9);
                R0 r0;
                R1 r1;
                R2 r2;
                R3 r3;
                R4 r4 = f(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, out r0, out r1, out r2, out r3);
                return SendResult(CurrentRequestID, r0, r1, r2, r3, r4);
            }

            public bool MakeRequest(ushort requestId, CScopeUQueue su)
            {
                if (su == null)
                    return MakeRequest(requestId);
                return MakeRequest(requestId, su.UQueue);
            }

            public virtual bool MakeRequest(ushort requestId, CUQueue UQueue)
            {
                if (UQueue == null)
                    return MakeRequest(requestId);
                if (UQueue.HeadPosition > 0)
                    return MakeRequest(requestId, UQueue.GetBuffer(), UQueue.GetSize());
                return MakeRequest(requestId, UQueue.m_bytes, UQueue.GetSize());
            }

            public bool MakeRequest(ushort requestId)
            {
                return MakeRequest(requestId, (byte[])null, (uint)0);
            }

            public virtual bool MakeRequest(ushort requestId, byte[] data, uint len)
            {
                if (data == null)
                    len = 0;
                else if (len > (uint)data.Length)
                    len = (uint)data.Length;
                unsafe
                {
                    fixed (byte* buffer = data)
                    {
                        return ServerCoreLoader.MakeRequest(m_sh, requestId, buffer, len);
                    }
                }
            }

            protected virtual uint SendResult(ushort reqId, byte[] data, uint len, uint offset)
            {
                ulong index = Random ? CurrentRequestIndex : ulong.MaxValue;
                if (data == null || len == 0 || offset >= (uint)data.LongLength)
                {
                    if (data == null)
                        data = new byte[1];
                    unsafe
                    {
                        fixed (byte* buffer = data)
                        {
                            if (index == ulong.MaxValue)
                                return ServerCoreLoader.SendReturnData(Handle, reqId, 0, buffer);
                            return ServerCoreLoader.SendReturnDataIndex(Handle, index, reqId, 0, buffer);
                        }
                    }
                }
                unsafe
                {
                    if (len + offset > (uint)data.LongLength)
                        len = (uint)data.LongLength - offset;
                    fixed (byte* buffer = &(data[offset]))
                    {
                        if (index == ulong.MaxValue)
                            return ServerCoreLoader.SendReturnData(Handle, reqId, len, buffer);
                        return ServerCoreLoader.SendReturnDataIndex(Handle, index, reqId, len, buffer);
                    }
                }
            }

            protected virtual uint SendResultIndex(ulong reqIndex, ushort reqId, byte[] data, uint len, uint offset)
            {
                if (data == null || len == 0 || offset >= (uint)data.LongLength)
                {
                    if (data == null)
                        data = new byte[1];
                    unsafe
                    {
                        fixed (byte* buffer = data)
                        {
                            return ServerCoreLoader.SendReturnDataIndex(Handle, reqIndex, reqId, 0, buffer);
                        }
                    }
                }
                unsafe
                {
                    if (len + offset > (uint)data.LongLength)
                        len = (uint)data.LongLength - offset;
                    fixed (byte* buffer = &(data[offset]))
                    {
                        return ServerCoreLoader.SendReturnDataIndex(Handle, reqIndex, reqId, len, buffer);
                    }
                }
            }

            protected virtual uint SendResult(ushort reqId, byte[] data, uint len)
            {
                if (data != null && len > (uint)data.Length)
                    len = (uint)data.Length;
                unsafe
                {
                    if (Random)
                    {
                        ulong index = CurrentRequestIndex;
                        fixed (byte* buffer = data)
                        {
                            return ServerCoreLoader.SendReturnDataIndex(Handle, index, reqId, len, buffer);
                        }
                    }
                    fixed (byte* buffer = data)
                    {
                        return ServerCoreLoader.SendReturnData(Handle, reqId, len, buffer);
                    }
                }
            }

            protected virtual uint SendResultIndex(ulong reqIndex, ushort reqId, byte[] data, uint len)
            {
                if (data != null && len > (uint)data.Length)
                    len = (uint)data.Length;
                unsafe
                {
                    fixed (byte* buffer = data)
                    {
                        return ServerCoreLoader.SendReturnDataIndex(Handle, reqIndex, reqId, len, buffer);
                    }
                }
            }

            protected uint SendResult(ushort reqId, CUQueue q)
            {
                if (q == null)
                    return SendResult(reqId);
                return SendResult(reqId, q.IntenalBuffer, q.GetSize(), q.HeadPosition);
            }

            protected uint SendResultIndex(ulong reqIndex, ushort reqId, CUQueue q)
            {
                if (q == null)
                    return SendResultIndex(reqIndex, reqId);
                return SendResultIndex(reqIndex, reqId, q.IntenalBuffer, q.GetSize(), q.HeadPosition);
            }

            protected uint SendResult(ushort reqId, CScopeUQueue su)
            {
                if (su == null || su.UQueue == null)
                    return SendResult(reqId);
                return SendResult(reqId, su.UQueue);
            }

            protected uint SendResultIndex(ulong reqIndex, ushort reqId, CScopeUQueue su)
            {
                if (su == null || su.UQueue == null)
                    return SendResultIndex(reqIndex, reqId);
                return SendResultIndex(reqIndex, reqId, su.UQueue);
            }

            protected uint SendResult(ushort reqId)
            {
                return SendResult(reqId, (byte[])null, (uint)0);
            }

            protected uint SendResultIndex(ulong reqIndex, ushort reqId)
            {
                return SendResultIndex(reqIndex, reqId, (byte[])null, (uint)0);
            }

            protected uint SendResultIndex<T0>(ulong reqIndex, ushort reqId, T0 t0)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0);
                uint res = SendResultIndex(reqIndex, reqId, su);
                CScopeUQueue.Unlock(su);
                return res;
            }

            protected uint SendResultIndex<T0, T1>(ulong reqIndex, ushort reqId, T0 t0, T1 t1)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1);
                uint res = SendResultIndex(reqIndex, reqId, su);
                CScopeUQueue.Unlock(su);
                return res;
            }

            protected uint SendResultIndex<T0, T1, T2>(ulong reqIndex, ushort reqId, T0 t0, T1 t1, T2 t2)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2);
                uint res = SendResultIndex(reqIndex, reqId, su);
                CScopeUQueue.Unlock(su);
                return res;
            }

            protected uint SendResultIndex<T0, T1, T2, T3>(ulong reqIndex, ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3);
                uint res = SendResultIndex(reqIndex, reqId, su);
                CScopeUQueue.Unlock(su);
                return res;
            }

            protected uint SendResultIndex<T0, T1, T2, T3, T4>(ulong reqIndex, ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                uint res = SendResultIndex(reqIndex, reqId, su);
                CScopeUQueue.Unlock(su);
                return res;
            }

            protected uint SendResult<T0>(ushort reqId, T0 t0)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0);
                uint res = SendResult(reqId, su);
                CScopeUQueue.Unlock(su);
                return res;
            }

            protected uint SendResult<T0, T1>(ushort reqId, T0 t0, T1 t1)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1);
                uint res = SendResult(reqId, su);
                CScopeUQueue.Unlock(su);
                return res;
            }

            protected uint SendResult<T0, T1, T2>(ushort reqId, T0 t0, T1 t1, T2 t2)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2);
                uint res = SendResult(reqId, su);
                CScopeUQueue.Unlock(su);
                return res;
            }

            protected uint SendResult<T0, T1, T2, T3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3);
                uint res = SendResult(reqId, su);
                CScopeUQueue.Unlock(su);
                return res;
            }

            protected uint SendResult<T0, T1, T2, T3, T4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                uint res = SendResult(reqId, su);
                CScopeUQueue.Unlock(su);
                return res;
            }

            internal override void OnChatComing(tagChatRequestID chatRequestID)
            {
                switch (chatRequestID)
                {
                    case tagChatRequestID.idSendUserMessageEx:
                        {
                            byte[] msg;
                            string userId;
                            bool endian = false;
                            tagOperationSystem os = ServerCoreLoader.GetPeerOs(m_sh, ref endian);
                            m_qBuffer.Endian = endian;
                            m_qBuffer.OS = os;
                            m_qBuffer.Load(out userId).Pop(out msg);
                            OnSendUserMessageEx(userId, msg);
                        }
                        break;
                    case tagChatRequestID.idSpeakEx:
                        {
                            uint size;
                            byte[] message;
                            bool endian = false;
                            tagOperationSystem os = ServerCoreLoader.GetPeerOs(m_sh, ref endian);
                            m_qBuffer.Endian = endian;
                            m_qBuffer.OS = os;
                            m_qBuffer.Load(out size).Pop(out message, size);
                            size = m_qBuffer.GetSize() / sizeof(uint);
                            uint[] groups = new uint[size];
                            for (uint n = 0; n < size; ++n)
                            {
                                m_qBuffer.Load(out groups[n]);
                            }
                            OnPublishEx(groups, message);
                        }
                        break;
                    default:
                        base.OnChatComing(chatRequestID);
                        break;
                }
            }
        }
    }
}
