/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <stack>
#include <et-ext/rt/raytraceobjects.h>

namespace et
{
namespace rt
{
	class ET_ALIGNED(16) KDTree
	{
	public:
		struct ET_ALIGNED(16) Node
		{
			float distance;
			int_fast32_t axis;
			index startIndex;
			index endIndex;
			index children[2];

			inline index numIndexes() const
				{ return endIndex - startIndex; }
		};

		struct Stats
		{
			size_t totalTriangles = 0;
			size_t totalNodes = 0;
			size_t maxDepth = 0;
			index distributedTriangles = 0;
			index leafNodes = 0;
			index emptyLeafNodes = 0;
			index maxTrianglesPerNode = 0;
			index minTrianglesPerNode = std::numeric_limits<index>::max();
		};
		
		struct ET_ALIGNED(16) TraverseResult
		{
			float4 intersectionPoint;
			float4 intersectionPointBarycentric;
			size_t triangleIndex = InvalidIndex;
		};

	public:
		~KDTree();
		
        void build(const TriangleList&, size_t maxDepth);
		Stats nodesStatistics() const;
		void cleanUp();
		
		const Node& root() const
			{ return _nodes.front(); }
		
		const Node& nodeAt(size_t i) const
			{ return _nodes.at(i); }

		const BoundingBox& bboxAt(size_t i) const
			{ return _boundingBoxes.at(i); }
		
		TraverseResult traverse(const Ray& r);
		void printStructure();
		
		const Triangle& triangleAtIndex(size_t) const;
		
	private:
		void printStructure(const Node&, const std::string&);
		
		Node buildRootNode();
		void splitNodeUsingSortedArray(size_t, size_t);
		void buildSplitBoxesUsingAxisAndPosition(size_t nodeIndex, int axis, float position);
		void distributeTrianglesToChildren(size_t nodeIndex);
		
	private:
		Vector<Node> _nodes;
		BoundingBoxList _boundingBoxes;
		TriangleList _triangles;
		IntersectionDataList _intersectionData;
		Vector<index> _indexes;
		size_t _maxDepth = 0;
		size_t _maxBuildDepth = 0;
	};
    
    template <size_t MaxElements, class T>
    struct ET_ALIGNED(16) FastStack
    {
    public:
        enum : size_t
        {
            MaxElementsPlusOne = MaxElements + 1,
        };
        
    public:
        template <typename ... Args>
        void emplace(Args&&... a)
        {
            ET_ASSERT(_size < MaxElements);
            _elements[_size] = T(std::forward<Args>(a)...);
            ++_size;
        }
        
        void push(const T& value)
        {
            ET_ASSERT(_size < MaxElements);
            _elements[_size] = value;
            ++_size;
        }
        
        bool empty() const
            { return _size == 0; }
        
        bool hasSomething() const
            { return _size > 0; }
        
        const T& top() const
            { ET_ASSERT(_size < MaxElementsPlusOne); return _elements[_size - 1]; }
        
        void pop()
            { ET_ASSERT(_size > 0); --_size; }
        
        size_t size() const
            { return _size; }
        
    private:
        T _elements[MaxElements];
        size_t _size = 0;
    };
}
}