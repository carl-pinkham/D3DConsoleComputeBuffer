
// d3d12.lib and d3dcompiler.lib defined in Project settings

#include "main.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

struct Data
{
    XMFLOAT3 v1;
    XMFLOAT2 v2;
};

std::ofstream& operator << (std::ofstream& os, Data d)
{
    os << "(" << d.v1.x << ", " << d.v1.y << ", " << d.v1.z << ", " << d.v2.x << ", " << d.v2.y << ")";
    return os;
}

std::ostream& operator << (std::ostream& os, Data d)
{
    os << "(" << d.v1.x << ", " << d.v1.y << ", " << d.v1.z << ", " << d.v2.x << ", " << d.v2.y << ")";
    return os;
}

ID3D12Device* D3D12Device;
ID3D12CommandQueue* D3D12CommandQueue;
ID3D12CommandAllocator* D3D12CommandAllocator;
ID3D12GraphicsCommandList* D3D12GraphicsCommandList;
ID3D12Fence* D3D12Fence;
UINT64 fenceValue{ 1 };
D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

void CloseExecuteWaitReset(ID3D12PipelineState* pipelineState);

int main()
{
    // D3D12CreateDevice(nullptr, featureLevel, __uuidof(ID3D12Device), reinterpret_cast<void**>(&D3D12Device));
    D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&D3D12Device));
    D3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&D3D12Fence));
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    D3D12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&D3D12CommandQueue));
    D3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&D3D12CommandAllocator));
    D3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12CommandAllocator, nullptr, IID_PPV_ARGS(&D3D12GraphicsCommandList));
    
    D3D12GraphicsCommandList->Close();
    D3D12CommandAllocator->Reset();
    D3D12GraphicsCommandList->Reset(D3D12CommandAllocator, nullptr);
    
    const int NumDataElements = 32;
    std::vector<Data> dataA(NumDataElements);
    std::vector<Data> dataB(NumDataElements);
    for (int i = 0; i < NumDataElements; ++i)
    {
        dataA[i].v1 = XMFLOAT3(i, i, i);
        dataA[i].v2 = XMFLOAT2(i, 0);
        
        dataB[i].v1 = XMFLOAT3(-i, i, 0.0f);
        dataB[i].v2 = XMFLOAT2(0, -i);
    }
    UINT64 byteSize = dataA.size() * sizeof(Data);

    ID3D12Resource* InputBufferA;
    CD3DX12_HEAP_PROPERTIES InputBufferAHP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC InputBufferARD = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    D3D12Device->CreateCommittedResource(
        &InputBufferAHP,
        D3D12_HEAP_FLAG_NONE,
        &InputBufferARD,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&InputBufferA));

    ID3D12Resource* InputUploadBufferA;
    CD3DX12_HEAP_PROPERTIES InputUploadBufferAHP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC InputUploadBufferARD = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    D3D12Device->CreateCommittedResource(
        &InputUploadBufferAHP,
        D3D12_HEAP_FLAG_NONE,
        &InputUploadBufferARD,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&InputUploadBufferA));

    D3D12_SUBRESOURCE_DATA subResourceData1 = {};
    subResourceData1.pData = dataA.data();
    subResourceData1.RowPitch = byteSize;
    subResourceData1.SlicePitch = subResourceData1.RowPitch;

    CD3DX12_RESOURCE_BARRIER InputBufferARB1 = CD3DX12_RESOURCE_BARRIER::Transition(InputBufferA, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    D3D12GraphicsCommandList->ResourceBarrier(1, &InputBufferARB1);
    UpdateSubresources<1>(D3D12GraphicsCommandList, InputBufferA, InputUploadBufferA, 0, 0, 1, &subResourceData1);
    CD3DX12_RESOURCE_BARRIER InputBufferARB2 = CD3DX12_RESOURCE_BARRIER::Transition(InputBufferA, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    D3D12GraphicsCommandList->ResourceBarrier(1, &InputBufferARB2);

    ID3D12Resource* InputBufferB;
    CD3DX12_HEAP_PROPERTIES InputBufferBHP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC InputBufferBRD = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    D3D12Device->CreateCommittedResource(
        &InputBufferBHP,
        D3D12_HEAP_FLAG_NONE,
        &InputBufferBRD,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&InputBufferB));

    ID3D12Resource* InputUploadBufferB;
    CD3DX12_HEAP_PROPERTIES InputUploadBufferBHP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC InputUploadBufferBRD = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    D3D12Device->CreateCommittedResource(
        &InputUploadBufferBHP,
        D3D12_HEAP_FLAG_NONE,
        &InputUploadBufferBRD,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&InputUploadBufferB));

    D3D12_SUBRESOURCE_DATA subResourceData2 = {};
    subResourceData2.pData = dataB.data();
    subResourceData2.RowPitch = byteSize;
    subResourceData2.SlicePitch = subResourceData2.RowPitch;

    CD3DX12_RESOURCE_BARRIER InputBufferBRB1 = CD3DX12_RESOURCE_BARRIER::Transition(InputBufferB, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    D3D12GraphicsCommandList->ResourceBarrier(1, &InputBufferBRB1);
    UpdateSubresources<1>(D3D12GraphicsCommandList, InputBufferB, InputUploadBufferB, 0, 0, 1, &subResourceData2);
    CD3DX12_RESOURCE_BARRIER InputBufferBRB2 = CD3DX12_RESOURCE_BARRIER::Transition(InputBufferB, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    D3D12GraphicsCommandList->ResourceBarrier(1, &InputBufferBRB2);

    ID3D12Resource* OutputBuffer;
    CD3DX12_HEAP_PROPERTIES OutputBufferHP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC OutputBufferRD = CD3DX12_RESOURCE_DESC::Buffer(byteSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    D3D12Device->CreateCommittedResource(
        &OutputBufferHP,
        D3D12_HEAP_FLAG_NONE,
        &OutputBufferRD,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&OutputBuffer));

    ID3D12Resource* ReadBackBuffer;
    CD3DX12_HEAP_PROPERTIES ReadBackBufferHP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
    CD3DX12_RESOURCE_DESC ReadBackBufferRD = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    D3D12Device->CreateCommittedResource(
        &ReadBackBufferHP,
        D3D12_HEAP_FLAG_NONE,
        &ReadBackBufferRD,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&ReadBackBuffer));

    ID3D12RootSignature* D3D12RootSignature;
    CD3DX12_ROOT_PARAMETER slotRootParameter[3];
    slotRootParameter[0].InitAsShaderResourceView(0);
    slotRootParameter[1].InitAsShaderResourceView(1);
    slotRootParameter[2].InitAsUnorderedAccessView(0);
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);
    ID3DBlob* serializedRootSig = nullptr;
    ID3DBlob* errorBlob = nullptr;
    D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
    if (errorBlob != nullptr)
    {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    D3D12Device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&D3D12RootSignature));

    UINT compileFlags = 0;
    ID3DBlob* D3D12Shader = nullptr;
    ID3DBlob* errors;
    D3DCompileFromFile(L"VecAdd.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CS", "cs_5_0", compileFlags, 0, &D3D12Shader, &errors);
    if (errors != nullptr)
        OutputDebugStringA((char*)errors->GetBufferPointer());

    ID3D12PipelineState* D3D12PSO;
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = D3D12RootSignature;
    computePsoDesc.CS =
    {
        reinterpret_cast<BYTE*>(D3D12Shader->GetBufferPointer()), D3D12Shader->GetBufferSize()
    };
    computePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    D3D12Device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&D3D12PSO));

    CloseExecuteWaitReset(D3D12PSO);

    D3D12GraphicsCommandList->SetComputeRootSignature(D3D12RootSignature);
    D3D12GraphicsCommandList->SetComputeRootShaderResourceView(0, InputBufferA->GetGPUVirtualAddress());
    D3D12GraphicsCommandList->SetComputeRootShaderResourceView(1, InputBufferB->GetGPUVirtualAddress());
    D3D12GraphicsCommandList->SetComputeRootUnorderedAccessView(2, OutputBuffer->GetGPUVirtualAddress());

    D3D12GraphicsCommandList->Dispatch(1, 1, 1);

    CD3DX12_RESOURCE_BARRIER OutputBufferRB1 = CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
    D3D12GraphicsCommandList->ResourceBarrier(1, &OutputBufferRB1);

    D3D12GraphicsCommandList->CopyResource(ReadBackBuffer, OutputBuffer);

    CD3DX12_RESOURCE_BARRIER OutputBufferRB2 = CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
    D3D12GraphicsCommandList->ResourceBarrier(1, &OutputBufferRB2);

    CloseExecuteWaitReset(nullptr);

    Data* mappedData = nullptr;
    ReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));

    std::ofstream fout("results.txt");

    for (int i = 0; i < NumDataElements; ++i)
    {
        fout << mappedData[i] << std::endl;
    }

    for (int i = 0; i < NumDataElements; ++i)
    {
        std::cout << mappedData[i] << std::endl;
    }

    fout.close();

    ReadBackBuffer->Unmap(0, nullptr);

    D3D12PSO->Release();
    // errors->Release();
    D3D12Shader->Release();
    // errorBlob->Release();
    serializedRootSig->Release();
    D3D12RootSignature->Release();
    OutputBuffer->Release();
    ReadBackBuffer->Release();
    InputUploadBufferB->Release();
    InputBufferB->Release();
    InputUploadBufferA->Release();
    InputBufferA->Release();
    D3D12Fence->Release();
    D3D12GraphicsCommandList->Release();
    D3D12CommandAllocator->Release();
    D3D12CommandQueue->Release();
    D3D12Device->Release();
}

void CloseExecuteWaitReset(ID3D12PipelineState* pipelineState)
{
    D3D12GraphicsCommandList->Close();
    ID3D12CommandList* commandList = { D3D12GraphicsCommandList };
    D3D12CommandQueue->ExecuteCommandLists(1, &commandList);
    fenceValue++;
    D3D12CommandQueue->Signal(D3D12Fence, fenceValue);
    if (D3D12Fence->GetCompletedValue() < fenceValue)
    {
        HANDLE fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        D3D12Fence->SetEventOnCompletion(fenceValue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
        CloseHandle(fenceEvent);
    }
    D3D12CommandAllocator->Reset();
    D3D12GraphicsCommandList->Reset(D3D12CommandAllocator, pipelineState);
}
