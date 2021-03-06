/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/core/tools.h>
#include <et/core/conversion.h>
#include <et/scene3d/mesh.h>
#include <et/scene3d/storage.h>

namespace et 
{

namespace s3d 
{

const std::string Mesh::defaultMeshName = "mesh";

Mesh::Mesh(const std::string& name, BaseElement* parent) :
	RenderableElement(name, parent), _undeformedTransformationMatrices(4, identityMatrix)
{
}

void Mesh::calculateSupportData()
{
	_supportData.boundingBox = BoundingBox();
	
	float processedBatches = 0.0f;
	vec3 minVertex( std::numeric_limits<float>::max());
	vec3 maxVertex(-std::numeric_limits<float>::max());
	
	for (auto& rb : renderBatches())
	{
		const auto& vs = rb->vertexStorage();
		const auto& ia = rb->indexArray();
		if (vs.valid() && ia.valid() && vs->hasAttributeWithType(VertexAttributeUsage::Position, DataType::Vec3))
		{
			rb->calculateBoundingBox();
			minVertex = minv(minVertex, rb->boundingBox().minVertex());
			maxVertex = maxv(maxVertex, rb->boundingBox().maxVertex());
			processedBatches += 1.0f;
		}
	}
	
	if (processedBatches > 0.0f)
	{
		vec3 dimensions = maxVertex - minVertex;
		_supportData.boundingBox = BoundingBox(0.5f * (minVertex + maxVertex), 0.5f * dimensions);
		_supportData.boundingSphereRadius =  0.5f * std::max(std::max(dimensions.x, dimensions.y), dimensions.z);
	}
}

Mesh* Mesh::duplicate()
{
	abort();

	/*
	 * TODO : implement
	 *
	Mesh* result = etCreateObject<Mesh>(name(), parent());
	result->_supportData = _supportData;
	result->_undeformedTransformationMatrices = _undeformedTransformationMatrices;
	
	for (auto batch : renderBatches())
		result->addRenderBatch(RenderBatch::Pointer(batch->duplicate()));

	// base object
	duplicateBasePropertiesToObject(result);
	duplicateChildrenToObject(result);
	
	return result;
	// */
	
	return nullptr;
}

void Mesh::transformInvalidated()
{
	_supportData.shouldUpdateBoundingBox = true;
	_supportData.shouldUpdateBoundingSphere = true;
	_supportData.shouldUpdateBoundingSphereUntransformed = true;
}

float Mesh::finalTransformScale()
{
	return 1.0f / std::pow(std::abs(finalInverseTransform().mat3().determinant()), 1.0f / 3.0f);
}

const Sphere& Mesh::boundingSphereUntransformed()
{
	if (_supportData.shouldUpdateBoundingSphereUntransformed)
	{
		_supportData.untranfromedBoundingSphere = Sphere(_supportData.boundingBox.center, _supportData.boundingSphereRadius);
		_supportData.shouldUpdateBoundingSphereUntransformed = false;
	}
	return _supportData.untranfromedBoundingSphere;
}

const Sphere& Mesh::boundingSphere()
{
	if (_supportData.shouldUpdateBoundingSphere)
	{
		const auto& ft = finalTransform();
		_supportData.tranfromedBoundingSphere = Sphere(ft * _supportData.boundingBox.center,
			finalTransformScale() * _supportData.boundingSphereRadius);
		_supportData.shouldUpdateBoundingSphere = false;
	}
	return _supportData.tranfromedBoundingSphere;
}

const BoundingBox& Mesh::boundingBox()
{
	return _supportData.boundingBox;
}

const BoundingBox& Mesh::tranformedBoundingBox()
{
	if (_supportData.shouldUpdateBoundingBox)
	{
		_supportData.transformedBoundingBox = _supportData.boundingBox.transform(finalTransform());
		_supportData.shouldUpdateBoundingBox = false;
	}
	
	return _supportData.transformedBoundingBox;
}

const Vector<mat4>& Mesh::deformationMatrices()
{
	if (_deformer.valid())
		return _deformer->calculateTransforms();
	
	for (uint32_t i = 0; i < 4; ++i)
		_undeformedTransformationMatrices[i] = finalTransform();
	
	return _undeformedTransformationMatrices;
}

/*
 * Bake deformations + stuff for it
 */

template <DataType attribType>
void copyAttributeWithType(VertexStorage::Pointer from, VertexStorage::Pointer to, VertexAttributeUsage attrib)
{
	auto c0 = from->accessData<attribType>(attrib, 0);
	auto c1 = to->accessData<attribType>(attrib, 0);
	for (uint32_t i = 0; i < from->capacity(); ++i)
		c1[i] = c0[i];
}

void copyAttribute(VertexStorage::Pointer from, VertexStorage::Pointer to, VertexAttributeUsage attrib)
{
	if (!from->hasAttribute(attrib)) return;
	
	switch (from->attributeType(attrib))
	{
		case DataType::Float:
			copyAttributeWithType<DataType::Float>(from, to, attrib);
			break;
		case DataType::Int:
			copyAttributeWithType<DataType::Int>(from, to, attrib);
			break;
		case DataType::Vec2:
			copyAttributeWithType<DataType::Vec2>(from, to, attrib);
			break;
		case DataType::Vec3:
			copyAttributeWithType<DataType::Vec3>(from, to, attrib);
			break;
		case DataType::Vec4:
			copyAttributeWithType<DataType::Vec4>(from, to, attrib);
			break;
		case DataType::IntVec2:
			copyAttributeWithType<DataType::IntVec2>(from, to, attrib);
			break;
		case DataType::IntVec3:
			copyAttributeWithType<DataType::IntVec3>(from, to, attrib);
			break;
		case DataType::IntVec4:
			copyAttributeWithType<DataType::IntVec4>(from, to, attrib);
			break;
		default:
			ET_FAIL("Unhandled attribute type");
	}
}

void copyVector3Rotated(VertexStorage::Pointer from, VertexStorage::Pointer to, VertexAttributeUsage attrib,
	const mat4& transform)
{
	if (!from->hasAttribute(attrib)) return;
	
	auto c0 = from->accessData<DataType::Vec3>(attrib, 0);
	auto c1 = to->accessData<DataType::Vec3>(attrib, 0);
	for (uint32_t i = 0; i < from->capacity(); ++i)
		c1[i] = transform.rotationMultiply(c0[i]);
}

void copyVector3Transformed(VertexStorage::Pointer from, VertexStorage::Pointer to, VertexAttributeUsage attrib,
	const mat4& transform)
{
	if (!from->hasAttribute(attrib)) return;
	
	auto c0 = from->accessData<DataType::Vec3>(attrib, 0);
	auto c1 = to->accessData<DataType::Vec3>(attrib, 0);
	for (uint32_t i = 0; i < from->capacity(); ++i)
		c1[i] = transform * c0[i];
}

void skinVector3Rotated(VertexStorage::Pointer from, VertexStorage::Pointer to, VertexAttributeUsage attrib,
	const Vector<mat4>& transforms)
{
	if (!from->hasAttribute(attrib)) return;
	
	auto c0 = from->accessData<DataType::Vec3>(attrib, 0);
	auto c1 = to->accessData<DataType::Vec3>(attrib, 0);
	auto bi = to->accessData<DataType::IntVec4>(VertexAttributeUsage::BlendIndices, 0);
	auto bw = to->accessData<DataType::Vec4>(VertexAttributeUsage::BlendWeights, 0);
	for (uint32_t i = 0; i < from->capacity(); ++i)
	{
		c1[i] =
			transforms[bi[i][0]].rotationMultiply(c0[i]) * bw[i][0] +
			transforms[bi[i][1]].rotationMultiply(c0[i]) * bw[i][1] +
			transforms[bi[i][2]].rotationMultiply(c0[i]) * bw[i][2] +
			transforms[bi[i][3]].rotationMultiply(c0[i]) * bw[i][3];
	}
}

void skinVector3Transformed(VertexStorage::Pointer from, VertexStorage::Pointer to, VertexAttributeUsage attrib,
	const Vector<mat4>& transforms)
{
	if (!from->hasAttribute(attrib)) return;
	
	auto c0 = from->accessData<DataType::Vec3>(attrib, 0);
	auto c1 = to->accessData<DataType::Vec3>(attrib, 0);
	auto bi = to->accessData<DataType::IntVec4>(VertexAttributeUsage::BlendIndices, 0);
	auto bw = to->accessData<DataType::Vec4>(VertexAttributeUsage::BlendWeights, 0);
	for (uint32_t i = 0; i < from->capacity(); ++i)
	{
		c1[i] = (transforms[bi[i][0]] * c0[i]) * bw[i][0] + (transforms[bi[i][1]] * c0[i]) * bw[i][1] +
			(transforms[bi[i][2]] * c0[i]) * bw[i][2] +  (transforms[bi[i][3]] * c0[i]) * bw[i][3];
	}
}

bool Mesh::skinned() const
{
	ET_FAIL("TODO");
	/*
	 * TODO
	 *
	return _vertexStorage->hasAttribute(VertexAttributeUsage::BlendIndices) &&
		_vertexStorage->hasAttribute(VertexAttributeUsage::BlendWeights);
	// */
	return false;
}

VertexStorage::Pointer Mesh::bakeDeformations()
{
	ET_FAIL("TODO");
	
	/*
	 * TODO
	 *
	const auto& transforms = deformationMatrices();
	
	VertexStorage::Pointer result =
		VertexStorage::Pointer::create(_vertexStorage->declaration(), _vertexStorage->capacity());
	
	copyAttribute(_vertexStorage, result, VertexAttributeUsage::Color);
	copyAttribute(_vertexStorage, result, VertexAttributeUsage::TexCoord0);
	copyAttribute(_vertexStorage, result, VertexAttributeUsage::TexCoord1);
	copyAttribute(_vertexStorage, result, VertexAttributeUsage::TexCoord2);
	copyAttribute(_vertexStorage, result, VertexAttributeUsage::TexCoord3);
	copyAttribute(_vertexStorage, result, VertexAttributeUsage::Smoothing);
	copyAttribute(_vertexStorage, result, VertexAttributeUsage::BlendIndices);
	copyAttribute(_vertexStorage, result, VertexAttributeUsage::BlendWeights);
	
	if (skinned())
	{
		skinVector3Transformed(_vertexStorage, result, VertexAttributeUsage::Position, transforms);
		skinVector3Rotated(_vertexStorage, result, VertexAttributeUsage::Normal, transforms);
		skinVector3Rotated(_vertexStorage, result, VertexAttributeUsage::Tangent, transforms);
		skinVector3Rotated(_vertexStorage, result, VertexAttributeUsage::Binormal, transforms);
	}
	else
	{
		const mat4& ft = finalTransform();
		copyVector3Transformed(_vertexStorage, result, VertexAttributeUsage::Position, ft);
		copyVector3Rotated(_vertexStorage, result, VertexAttributeUsage::Normal, ft);
		copyVector3Rotated(_vertexStorage, result, VertexAttributeUsage::Tangent, ft);
		copyVector3Rotated(_vertexStorage, result, VertexAttributeUsage::Binormal, ft);
	}
	return result;
	// */
	return VertexStorage::Pointer();
}

RayIntersection Mesh::intersectsWorldSpaceRay(const ray3d& ray)
{
	RayIntersection result;
	mat4 invTransform = transform().inverted();
	ray3d localRay(invTransform * ray.origin, invTransform.rotationMultiply(ray.direction));
	for (const RenderBatch::Pointer& rb : renderBatches())
	{
		RayIntersection batchIntersection = rb->intersectsLocalSpaceRay(localRay);
		if (batchIntersection.occurred && (batchIntersection.time < result.time))
		{
			result.occurred = true;
			result.time = batchIntersection.time;
		}
	}

	if (result.occurred)
	{
		vec3 worldSpacePos = localRay.origin + result.time * localRay.direction;
		worldSpacePos = transform() * worldSpacePos;
		result.time = (worldSpacePos - ray.origin).length();
	}

	return result;
}

}

}
