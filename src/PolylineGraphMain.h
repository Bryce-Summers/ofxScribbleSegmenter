#pragma once

/*
* One Stop Include File.
*
* Written by Bryce Summers on 8/18/2016
*
* This header loads all of the necessary files for working with Polyline Graph Embeddings.
*/

// Original Single Purpose Embedding and curve offsetting.
#include "FaceFinder.h"
#include "OffsetCurves.h"

// General purpose Half edge mesh / graph representation,
// which may be used to easily implement a variety of sophisticated operations.
#include "HalfedgeGraph.h"

// Application specific classes for working with embedded Polyline graphs.
// Since we control the embedding in house,
// we get to add as much secret sauce and useful invariants and extra information to the data
// to suit our algorithmic and processing needs.
#include "PolylineGraphEmbedder.h"
#include "PolylineGraphData.h"
#include "PolylineGraphPostProcessor.h"