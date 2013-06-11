/* $Id$ */

/*
 * This file is part of FreeRCT.
 * FreeRCT is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * FreeRCT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with FreeRCT. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file coaster.cpp Coaster type data. */

#include "stdafx.h"
#include "sprite_store.h"
#include "coaster.h"
#include "fileio.h"

TrackVoxel::TrackVoxel()
{
	for (uint i = 0; i < lengthof(this->back); i++) this->back[i] = NULL;
	for (uint i = 0; i < lengthof(this->front); i++) this->front[i] = NULL;
}

TrackVoxel::~TrackVoxel()
{
	/* Images are deleted by the Rcd manager. */
}

/**
 * Load a track voxel.
 * @param rcd_file Data file being loaded.
 * @param length Length of the voxel (according to the file).
 * @param sprites Already loaded sprite blocks.
 * @return Loading was successful.
 */
bool TrackVoxel::Load(RcdFile *rcd_file, size_t length, const ImageMap &sprites)
{
	if (length != 4*4 + 4*4 + 3 + 1) return false;
	for (uint i = 0; i < 4; i++) {
		if (!LoadSpriteFromFile(rcd_file, sprites, &this->back[i])) return false;
	}
	for (uint i = 0; i < 4; i++) {
		if (!LoadSpriteFromFile(rcd_file, sprites, &this->front[i])) return false;
	}
	this->dx = rcd_file->GetInt8();
	this->dy = rcd_file->GetInt8();
	this->dz = rcd_file->GetInt8();
	this->space = rcd_file->GetUInt8();
	return true;
}

TrackPiece::TrackPiece() : RefCounter()
{
	this->voxel_count = 0;
	this->track_voxels = NULL;
}

TrackPiece::~TrackPiece()
{
	delete[] this->track_voxels;
}

/**
 * Load a track piece.
 * @param rcd_file Data file being loaded.
 * @param length Length of the voxel (according to the file).
 * @param sprites Already loaded sprite blocks.
 * @return Loading was successful.
 */
bool TrackPiece::Load(RcdFile *rcd_file, uint32 length, const ImageMap &sprites)
{
	if (length < 2+3+1+1+4+2) return false;
	length -= 2+3+1+1+4+2;
	this->entry_connect = rcd_file->GetUInt8();
	this->exit_connect = rcd_file->GetUInt8();
	this->exit_dx = rcd_file->GetInt8();
	this->exit_dy = rcd_file->GetInt8();
	this->exit_dz = rcd_file->GetInt8();
	this->speed = rcd_file->GetInt8();
	this->track_flags = rcd_file->GetUInt8();
	this->cost = rcd_file->GetUInt32();
	this->voxel_count = rcd_file->GetUInt16();
	if (length != 36u * this->voxel_count) return false;
	this->track_voxels = new TrackVoxel[this->voxel_count];
	for (int i = 0; i < this->voxel_count; i++) {
		if (!this->track_voxels[i].Load(rcd_file, 36, sprites)) return false;
	}
	return true;
}

#include "table/coasters_strings.cpp"

CoasterType::CoasterType() : RideType(RTK_COASTER)
{
	this->piece_count = 0;
	this->pieces = NULL;
}

/* virtual */ CoasterType::~CoasterType()
{
	delete[] this->pieces;
}

/**
 * Load a coaster type.
 * @param rcd_file Data file being loaded.
 * @param length Length of the voxel (according to the file).
 * @param texts Already loaded text blocks.
 * @param piece_map Already loaded track pieces.
 * @return Loading was successful.
 */
bool CoasterType::Load(RcdFile *rcd_file, uint32 length, const TextMap &texts, const TrackPiecesMap &piece_map)
{
	if (length < 2+1+4+2) return false;
	length -= 2+1+4+2;
	this->coaster_kind = rcd_file->GetUInt16();
	this->platform_type = rcd_file->GetUInt8();
	if (this->coaster_kind == 0 || this->coaster_kind >= CST_COUNT) return false;
	if (this->platform_type == 0 || this->platform_type >= CPT_COUNT) return false;

	TextData *text_data;
	if (!LoadTextFromFile(rcd_file, texts, &text_data)) return false;
	StringID base = _language.RegisterStrings(*text_data, _coasters_strings_table);
	this->SetupStrings(text_data, base, STR_GENERIC_COASTER_START, COASTERS_STRING_TABLE_END, COASTERS_NAME_TYPE, COASTERS_DESCRIPTION_TYPE);

	this->piece_count = rcd_file->GetUInt16();
	if (length != 4u * this->piece_count) return false;

	this->pieces = new TrackPieceRef[this->piece_count];
	for (int i = 0; i < this->piece_count; i++) {
		uint32 val = rcd_file->GetUInt32();
		if (val == 0) return false; // We don't expect missing track pieces (they should not be included at all).
		TrackPiecesMap::const_iterator iter = piece_map.find(val);
		if (iter == piece_map.end()) return false;
		this->pieces[i] = (*iter).second;
	}
	return true;
}

/* virtual */ RideInstance *CoasterType::CreateInstance() const
{
	assert(false); // XXX
	return NULL;
}

/* virtual */ const ImageData *CoasterType::GetView(uint8 orientation) const
{
	assert(false); // XXX
	return NULL;
}

/* virtual */ const StringID *CoasterType::GetInstanceNames() const
{
	assert(false); // XXX
	return NULL;
}