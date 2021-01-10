/*****************************************************************************
 * Copyright (c) 2013-2016 Intel Corporation
 * All rights reserved.
 *
 * WARRANTY DISCLAIMER
 *
 * THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
 * MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Intel Corporation is the author of the Materials, and requests that all
 * problem reports or change requests be submitted to it directly
 	for (int i = 0; i < num; i++) {
		if (i == index) continue;
		float d0 =  tx[index] - tx[i],d1 = ty[index] - ty[i], d2 = tz[index] - tz[i];
		float dist = sqrt(d0 * d0 + d1 * d1 + d2 * d2);
		float collideDist = r[index] + r[i];
		if (dist < collideDist && dist != 0) {
			float norm0 =  d0 / dist,norm1 =  d1 / dist,norm2 = d2 / dist;
			float dv0 =  vx[index] - vx[i], dv1 = vy[index] - vy[i],dv2 = vz[index] - vz[i];
			float vDotNorm = dv0 * norm0 + dv1 * norm1 + dv2 * norm2;

			float tanv0 =  dv0 - vDotNorm * norm0, tanv1 = dv1 - vDotNorm * norm1 , tanv2 = dv2 - vDotNorm * norm2;

			float factor = -0.005 * (collideDist - dist);
			force0 += factor * norm0 + B * dv0 + C * tanv0;
			force1 += factor * norm1 + B * dv1 + C * tanv1;
			force2 += factor * norm2 + B * dv2 + C * tanv2;
			
		}
	}
		if (ntx[index] < -xBound + r[index]) {
		ntx[index] = -xBound + r[index];
		nvx[index] *= Decay;
	}
	if (ntx[index] > xBound - r[index]) {
		ntx[index] = xBound - r[index];
		nvx[index] *= Decay;
	}
	if (nty[index] < -yBound + r[index]) {
		nty[index] = -yBound + r[index];
		nvy[index] *= Decay;
	}
	if (nty[index] > yBound - r[index]) {
		nty[index] = yBound - r[index];
		nvy[index] *= Decay;
	}
	if (ntz[index] < -zBound + r[index]) {
		ntz[index] = -zBound + r[index];
		nvz[index] *= Decay;
	}
	if (ntz[index] > zBound - r[index]) {
		ntz[index] = zBound - r[index];
		nvz[index] *= Decay;
	}

	
	ntx[index] = tx[index] + nvx[index] * interval;
	nty[index] = ty[index] + nvy[index] * interval;
	ntz[index] = tz[index] + nvz[index] * interval;

		const float G = -0.0003, B = 0.0002, C = 0.0001, interval = 0.05;
	const float xBound = 1.0, yBound = 1.0, zBound = 1.0;
	const float Decay = -0.5;
	float force0= 0.0, force1 = 0.0, force2 = 0.0;
 *****************************************************************************/
 
__kernel void updateSphere(
			__global float* tx, __global float* ty, __global float* tz, 
            __global float* vx, __global float* vy, __global float* vz, 
			__global float* r, __global float* m,
			__global float* ntx, __global float* nty, __global float* ntz, 
            __global float* nvx, __global float* nvy, __global float* nvz,
			uint num
			)
{
    const int index = get_global_id(0);
	const float G = -0.0003, A = 0.005, B = 0.002, C = 0.001, interval = 0.5, M = 2;
	const float xBound = 1.0, yBound = 1.0, zBound = 1.0;
	const float Decay = -0.5;
	float force0= 0.0, force1 = 0.0, force2 = 0.0;
	for (int i = 0; i < num; i++) {
		if (i == index) continue;
		float d0 =  tx[index] - tx[i],d1 = ty[index] - ty[i], d2 = tz[index] - tz[i];
		float dist = sqrt(d0 * d0 + d1 * d1 + d2 * d2);
		float collideDist = r[index] + r[i];
		if (dist < collideDist) {
			float norm0 =  d0 / dist,norm1 =  d1 / dist,norm2 = d2 / dist;
			float dv0 =  vx[index] - vx[i], dv1 = vy[index] - vy[i],dv2 = vz[index] - vz[i];
			float vDotNorm = dv0 * norm0 + dv1 * norm1 + dv2 * norm2;

			float tanv0 =  dv0 - vDotNorm * norm0, tanv1 = dv1 - vDotNorm * norm1 , tanv2 = dv2 - vDotNorm * norm2;

			float factor = -A * (collideDist - dist);
			force0 += factor * norm0 + B * dv0 + C * tanv0;
			force1 += factor * norm1 + B * dv1 + C * tanv1;
			force2 += factor * norm2 + B * dv2 + C * tanv2;
			
		}
	}

	nvx[index] = vx[index] + force0 / M;
	nvy[index] = vy[index] + force1 / M + G * interval;
	nvz[index] = vz[index] + force2 / M;

	ntx[index] = tx[index] + nvx[index] * interval;
	nty[index] = ty[index] + nvy[index] * interval;
	ntz[index] = tz[index] + nvz[index] * interval;

	if (ntx[index] < -xBound + r[index]) {
		ntx[index] = -xBound + r[index];
		nvx[index] *= Decay;
	}
	if (ntx[index] > xBound - r[index]) {
		ntx[index] = xBound - r[index];
		nvx[index] *= Decay;
	}
	if (nty[index] < -yBound + r[index]) {
		nty[index] = -yBound + r[index];
		nvy[index] *= Decay;
	}
	if (nty[index] > yBound - r[index]) {
		nty[index] = yBound - r[index];
		nvy[index] *= Decay;
	}
	if (ntz[index] < -zBound + r[index]) {
		ntz[index] = -zBound + r[index];
		nvz[index] *= Decay;
	}
	if (ntz[index] > zBound - r[index]) {
		ntz[index] = zBound - r[index];
		nvz[index] *= Decay;
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	tx[index] = ntx[index];
	ty[index] = nty[index];
	tz[index] = ntz[index];
	vx[index] = nvx[index];
	vy[index] = nvy[index];
	vz[index] = nvz[index];
}
