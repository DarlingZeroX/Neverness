/**
 * @file NNAssetFormatTest.cpp
 * @brief .nnasset 二进制格式定义单元测试（Phase 8）。
 */

#include <gtest/gtest.h>

#include "NNAssetFormat.h"

using namespace NN::Runtime::Asset;

TEST(NNAssetFormatTest, HeaderSizeIs96)
{
	EXPECT_EQ(sizeof(NNAssetHeader), 96u)
		<< "NNAssetHeader 应为 96 字节（两行 cache line）";
}

TEST(NNAssetFormatTest, HeaderSizeMacro)
{
	EXPECT_EQ(NN_ASSET_HEADER_SIZE, 96u)
		<< "NN_ASSET_HEADER_SIZE 应为 96";
}

TEST(NNAssetFormatTest, BlobDescriptorSizeIs32)
{
	EXPECT_EQ(sizeof(NNBlobDescriptor), 32u)
		<< "NNBlobDescriptor 应为 32 字节";
}

TEST(NNAssetFormatTest, Align64_RoundUp)
{
	EXPECT_EQ(NNAssetAlign(0), 0u);
	EXPECT_EQ(NNAssetAlign(1), 64u);
	EXPECT_EQ(NNAssetAlign(63), 64u);
	EXPECT_EQ(NNAssetAlign(64), 64u);
	EXPECT_EQ(NNAssetAlign(65), 128u);
	EXPECT_EQ(NNAssetAlign(128), 128u);
	EXPECT_EQ(NNAssetAlign(1000), 1024u);
}

TEST(NNAssetFormatTest, HeaderValid_CorrectMagicAndVersion)
{
	NNAssetHeader header{};
	header.magic = NN_ASSET_MAGIC;
	header.version = NN_ASSET_VERSION;
	EXPECT_NE(NNAssetHeaderIsValid(&header), 0);
}

TEST(NNAssetFormatTest, HeaderInvalid_NullPointer)
{
	EXPECT_EQ(NNAssetHeaderIsValid(nullptr), 0);
}

TEST(NNAssetFormatTest, HeaderInvalid_BadMagic)
{
	NNAssetHeader header{};
	header.magic = 0x41424344u; /* "ABCD" */
	header.version = NN_ASSET_VERSION;
	EXPECT_EQ(NNAssetHeaderIsValid(&header), 0);
}

TEST(NNAssetFormatTest, HeaderInvalid_BadVersion)
{
	NNAssetHeader header{};
	header.magic = NN_ASSET_MAGIC;
	header.version = 999u;
	EXPECT_EQ(NNAssetHeaderIsValid(&header), 0);
}

TEST(NNAssetFormatTest, HeaderFieldOffsets)
{
	/* 验证关键字段在结构体中的偏移量 */
	NNAssetHeader header{};

	/* magic 在偏移 0 */
	EXPECT_EQ(offsetof(NNAssetHeader, magic), 0u);
	/* version 在偏移 4 */
	EXPECT_EQ(offsetof(NNAssetHeader, version), 4u);
	/* assetGuid 在偏移 8 */
	EXPECT_EQ(offsetof(NNAssetHeader, assetGuid), 8u);
	/* typeId 在偏移 24 */
	EXPECT_EQ(offsetof(NNAssetHeader, typeId), 24u);
	/* flags 在偏移 72 */
	EXPECT_EQ(offsetof(NNAssetHeader, flags), 72u);
}
