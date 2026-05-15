#include "MeshGenerator.h"
#include "CoreTypes.h"
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <Poly_Triangulation.hxx>
#include <TopoDS_Shape.hxx>
#include <Standard_Failure.hxx>
#include <exception>
#include <string.h>
#include <vector>
#include <array>

struct StepModelWrapper {
    TopoDS_Shape shape;
};

// Extracted Mesh Data wrapper
struct MeshDataWrapper {
    struct MeshPart {
        std::vector<std::array<double, 3>> vertices;
        std::vector<std::array<uint32_t, 3>> indices;
    };
    std::vector<MeshPart> parts;
};

extern "C" {

void GenerateMesh(HStepModel model, MeshSettings settings, HMeshData* outMesh, ExportResult* outResult) {
    if (!model || !outMesh || !outResult) return;

    try {
        StepModelWrapper* stepModel = static_cast<StepModelWrapper*>(model);
        
        // Protect against invalid settings
        double defl = settings.deflection > 0.0 ? settings.deflection : 0.1;
        double ang = settings.angle > 0.0 ? settings.angle : 0.5;

        // Execute OpenCASCADE meshing algorithm
        BRepMesh_IncrementalMesh meshGen(stepModel->shape, defl, settings.relativeMesh, ang);
        meshGen.Perform();
        
        MeshDataWrapper* wrapper = new MeshDataWrapper();
        
        // Extract the vertices and triangles from the OCC shape
        for (TopExp_Explorer ex(stepModel->shape, TopAbs_FACE); ex.More(); ex.Next()) {
            const TopoDS_Face& face = TopoDS::Face(ex.Current());
            TopLoc_Location loc;
            Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
            if (tri.IsNull() || tri->NbTriangles() == 0) continue;

            const bool reversed = (face.Orientation() == TopAbs_REVERSED);
            const int nbNodes = tri->NbNodes();
            const int nbTris = tri->NbTriangles();

            MeshDataWrapper::MeshPart part;
            part.vertices.resize(nbNodes);
            for (int i = 1; i <= nbNodes; ++i) {
                gp_Pnt p = tri->Node(i).Transformed(loc);
                part.vertices[i - 1] = { p.X(), p.Y(), p.Z() };
            }

            part.indices.resize(nbTris);
            for (int i = 1; i <= nbTris; ++i) {
                int n1, n2, n3;
                tri->Triangle(i).Get(n1, n2, n3);
                if (reversed) std::swap(n2, n3);
                part.indices[i - 1] = { static_cast<uint32_t>(n1 - 1),
                                        static_cast<uint32_t>(n2 - 1),
                                        static_cast<uint32_t>(n3 - 1) };
            }
            wrapper->parts.push_back(std::move(part));
        }
        
        *outMesh = static_cast<HMeshData>(wrapper);
        outResult->code = RESULT_SUCCESS;
        outResult->errorMessage[0] = '\0';
    } 
    catch (Standard_Failure& e) {
        outResult->code = RESULT_MESH_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, e.GetMessageString());
        *outMesh = nullptr;
    }
    catch (const std::exception& e) {
        outResult->code = RESULT_MESH_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, e.what());
        *outMesh = nullptr;
    }
    catch (...) {
        outResult->code = RESULT_UNKNOWN_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, "Unknown exception caught during meshing.");
        *outMesh = nullptr;
    }
}

void FreeMesh(HMeshData mesh) {
    if (mesh) {
        MeshDataWrapper* wrapper = static_cast<MeshDataWrapper*>(mesh);
        delete wrapper;
    }
}

} // extern "C"
