#include <iostream>
#include "voxel_stream_fast_noise_simd.h"

void VoxelStreamFastNoiseSIMD::set_channel(VoxelBuffer::ChannelId channel) {
	ERR_FAIL_INDEX(channel, VoxelBuffer::MAX_CHANNELS);
	_channel = channel;
}

VoxelBuffer::ChannelId VoxelStreamFastNoiseSIMD::get_channel() const {
	return _channel;
}

void VoxelStreamFastNoiseSIMD::set_noise(Ref<FastNoiseSIMD> noise) {
	_noise = noise;
}

Ref<FastNoiseSIMD> VoxelStreamFastNoiseSIMD::get_noise() const {
	return _noise;
}

void VoxelStreamFastNoiseSIMD::set_max_lod(int lod){
	_max_lod = lod;
}

int VoxelStreamFastNoiseSIMD::get_max_lod() const {
	return _max_lod;
}

void VoxelStreamFastNoiseSIMD::set_height_start(real_t y) {
	_height_start = y;
}

real_t VoxelStreamFastNoiseSIMD::get_height_start() const {
	return _height_start;
}

void VoxelStreamFastNoiseSIMD::set_height_range(real_t hrange) {
	_height_range = hrange;
}

real_t VoxelStreamFastNoiseSIMD::get_height_range() const {
	return _height_range;
}

void VoxelStreamFastNoiseSIMD::emerge_block(Ref<VoxelBuffer> out_buffer, Vector3i origin_in_voxels, int lod) {

	ERR_FAIL_COND(out_buffer.is_null());
	//ERR_FAIL_COND(_noise.is_null());
	//ERR_FAIL_COND(_fnoise.is_null());

	//OpenSimplexNoise& noise = **_noise;
	FastNoiseSIMD& noise = **_noise;
	VoxelBuffer& buffer = **out_buffer;

/*	if (origin_in_voxels.y > _height_start + _height_range) {

		if (_channel == VoxelBuffer::CHANNEL_SDF) {
			buffer.clear_channel_f(_channel, 100.0);
		}
		else if (_channel == VoxelBuffer::CHANNEL_TYPE) {
			buffer.clear_channel(_channel, 0);
		}

	}
	else if (origin_in_voxels.y + (buffer.get_size().y << lod) < _height_start) {

		if (_channel == VoxelBuffer::CHANNEL_SDF) {
			buffer.clear_channel_f(_channel, -100.0);
		}
		else if (_channel == VoxelBuffer::CHANNEL_TYPE) {
			buffer.clear_channel(_channel, 1);
		}

	}
	else*/ {

		// TODO Proper noise optimization
		// Prefetching was much faster, but it introduced LOD inconsistencies into the data itself, causing cracks.
		// Need to implement it properly, or use a different noise library.

		/*FloatBuffer3D &noise_buffer = _noise_buffer;
		const int noise_buffer_step = 2;

		Vector3i noise_buffer_size = buffer.get_size() / noise_buffer_step + Vector3i(1);
		if (noise_buffer.get_size() != noise_buffer_size) {
			noise_buffer.create(noise_buffer_size);
		}

		// Cache noise at lower grid resolution and interpolate after, much cheaper
		for (int z = 0; z < noise_buffer.get_size().z; ++z) {
			for (int x = 0; x < noise_buffer.get_size().x; ++x) {
				for (int y = 0; y < noise_buffer.get_size().y; ++y) {

					float lx = origin_in_voxels.x + (x << lod) * noise_buffer_step;
					float ly = origin_in_voxels.y + (y << lod) * noise_buffer_step;
					float lz = origin_in_voxels.z + (z << lod) * noise_buffer_step;

					float n = noise.get_noise_3d(lx, ly, lz);

					noise_buffer.set(x, y, z, n);
				}
			}
		}*/


		/*
		//////////////////////////////////////////
		// Looking up one voxel at a time works fine
		//float iso_scale = noise.get_period() * 0.1;
		float iso_scale = 6.4;
		//float noise_buffer_scale = 1.f / static_cast<float>(noise_buffer_step);

		Vector3i size = buffer.get_size();
		std::cout << "Noise origin(" << origin_in_voxels.x << ", " << origin_in_voxels.y << ", " << origin_in_voxels.z << ") size: (" << size.x << "," << size.y << "," << size.z << "), LOD: " << lod << std::endl;

		static long counter = 0;

		for (int z = 0; z < size.z; ++z) {
			for (int x = 0; x < size.x; ++x) {
				for (int y = 0; y < size.y; ++y) {

					float lx = origin_in_voxels.x + (x << lod);
					float ly = origin_in_voxels.y + (y << lod);
					float lz = origin_in_voxels.z + (z << lod);

					// We are near the isosurface, need to calculate noise value
					float n = noise.get_noise_3d(lx, ly, lz);
					//float n = noise_buffer.get_trilinear(x * noise_buffer_scale, y * noise_buffer_scale, z * noise_buffer_scale);
					//std::cout << "Noise value (" << lx << "," << ly << "," << lz << "): " << n << std::endl;

					if (counter++ % 1000 == 0) {
						//int dx = x << lod;
						//int dy = y << lod;
						//int dz = z << lod;
						printf("\toffset: (%d + [%d << %d]:%d, %d + [%d << %d]:%d, %d + [%d << %d]:%d) value: %.3f\n",
							origin_in_voxels.x, x, lod, (x<<lod),
							origin_in_voxels.y, y, lod, (y<<lod),
							origin_in_voxels.z, z, lod, (z<<lod),
							n
						);
						//std::cout << "Noise: (" << (x << lod) << ", " << (y << lod) << ", " << (z << lod) << ") value: (" << lx << "," << ly << "," << lz << "): " << n << std::endl;
						//std::cout << "Noise lod offset: (" << (x << lod) << ", " << (y << lod) << ", " << (z << lod) << ") value: (" << lx << "," << ly << "," << lz << "): " << n << std::endl;
					}

					// Apply height bias
					float t = (ly - _height_start) / _height_range;
					float bias = 2.0 * t - 1.0;
					float d = (n + bias) * iso_scale;

					// Set voxel
					if (_channel == VoxelBuffer::CHANNEL_SDF) {
						buffer.set_voxel_f(d, x, y, z, _channel);
					}
					else if (_channel == VoxelBuffer::CHANNEL_TYPE && d < 0) {
						buffer.set_voxel(1, x, y, z, _channel);
					}
				}
			}
		}

		//////////////////////////////////////////
		*/


		/*
		///////////////////////////////////////////
		// Looking up a whole set at once with lod scaled buffers works, but is incredibly slow and consumes 1GB of memory

		Vector3i size = buffer.get_size();
		size.x <<= lod;
		size.y <<= lod;
		size.z <<= lod;

		float* noise_set = noise.get_noise_set_3d(origin_in_voxels.x, origin_in_voxels.y, origin_in_voxels.z, size.x, size.y, size.z, 1.0);

		for (int x = 0, i = 0; x < size.x; x += (1 << lod)) {
			for (int y = 0; y < size.y; y += (1 << lod)) {
				for (int z = 0; z < size.z; z += (1 << lod), i++) {

					float n = noise_set[x * size.y * size.z + y * size.z + z];

					// Apply height bias
					//float t = (ly - _height_start) / _height_range;
					float bias = 0;// 2.0 * t - 1.0;
					float d = (n + bias);

					// Set voxel
					if (_channel == VoxelBuffer::CHANNEL_SDF) {
						buffer.set_voxel_f(d, x >> lod, y >> lod, z >> lod, _channel);
					}
					else if (_channel == VoxelBuffer::CHANNEL_TYPE && d < 0) {
						buffer.set_voxel(1, x >> lod, y >> lod, z >> lod, _channel);
					}
				}
			}
		}

		noise.free_noise_set(noise_set);
		////////////////////////////////////////////////////////
		*/
		


		
		/////////////////////////////
		// Looking up a whole set at once
		Vector3i size = buffer.get_size();
		float iso_scale = 1.0;// 6.4;
		//Vector3 orig_scale = noise.get_scale();
		//noise.set_scale(noise.get_scale() / float(1 << lod) );
		float* noise_set = noise.get_noise_set_3d(origin_in_voxels.x>>lod, origin_in_voxels.y>>lod, origin_in_voxels.z>>lod, size.x, size.y, size.z, 1<<lod);

		for (int x = 0, i=0; x < size.x; x++) {
			for (int y = 0;	y < size.y;	y++)   {
				for (int z = 0; z < size.z; z++, i++) {

					//float n = noise_set[x * size.y * size.z + y * size.z + z];
					float n = noise_set[i];

					// Apply height bias
					//float t = (ly - _height_start) / _height_range;
					float bias = 0;// 2.0 * t - 1.0;
					float d = (n + bias) * iso_scale;

					// Set voxel
					if (_channel == VoxelBuffer::CHANNEL_SDF) {
						buffer.set_voxel_f(d, x, y, z, _channel);
					}
					else if (_channel == VoxelBuffer::CHANNEL_TYPE && d < 0) {
						buffer.set_voxel(1, x, y, z, _channel);
					}
				}
			}
		}

		noise.free_noise_set(noise_set);

	}
}

void VoxelStreamFastNoiseSIMD::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_channel", "channel"), &VoxelStreamFastNoiseSIMD::set_channel);
	ClassDB::bind_method(D_METHOD("get_channel"), &VoxelStreamFastNoiseSIMD::get_channel);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "channel", PROPERTY_HINT_ENUM, VoxelBuffer::CHANNEL_ID_HINT_STRING), "set_channel", "get_channel");

	ClassDB::bind_method(D_METHOD("set_max_lod", "max_lod"), &VoxelStreamFastNoiseSIMD::set_max_lod);
	ClassDB::bind_method(D_METHOD("get_max_lod"), &VoxelStreamFastNoiseSIMD::get_max_lod);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_lod"), "set_max_lod", "get_max_lod");

	ClassDB::bind_method(D_METHOD("set_noise", "noise"), &VoxelStreamFastNoiseSIMD::set_noise);
	ClassDB::bind_method(D_METHOD("get_noise"), &VoxelStreamFastNoiseSIMD::get_noise);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "FastNoiseSIMD"), "set_noise", "get_noise");

	ClassDB::bind_method(D_METHOD("set_height_start", "hstart"), &VoxelStreamFastNoiseSIMD::set_height_start);
	ClassDB::bind_method(D_METHOD("get_height_start"), &VoxelStreamFastNoiseSIMD::get_height_start);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "height_start"), "set_height_start", "get_height_start");

	ClassDB::bind_method(D_METHOD("set_height_range", "hrange"), &VoxelStreamFastNoiseSIMD::set_height_range);
	ClassDB::bind_method(D_METHOD("get_height_range"), &VoxelStreamFastNoiseSIMD::get_height_range);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "height_range"), "set_height_range", "get_height_range");

}
