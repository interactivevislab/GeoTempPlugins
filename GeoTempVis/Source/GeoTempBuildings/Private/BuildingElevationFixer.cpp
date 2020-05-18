#include "BuildingElevationFixer.h"

USinglePartFixer* USinglePartFixer::Init(UBuildingPartComponent* inPart, TScriptInterface<IElevationProvider> provider)
{
	USinglePartFixer* fixer = NewObject<USinglePartFixer>();
	fixer->Part = inPart;
	fixer->AwaitingRequestCount = 0;
	for (auto contour : inPart->Outer)
	{
		for (auto point : contour.Points)
		{
			auto request = Cast<IElevationProvider>(provider.GetObject())->RequestElevation_Implementation(point);
			request->Callback.AddDynamic(fixer, &USinglePartFixer::ProcessRequest);
			fixer->Requests.Add(request);
			fixer->AwaitingRequestCount++;
		}
	}
	return fixer;
}

void USinglePartFixer::FixBuilding()
{
	float min = TNumericLimits<float>::Max();
	float max = TNumericLimits<float>::Lowest();
	for (auto height : heights)
	{
		if (min > height) min = height;
		if (max < height) max = height;
	}
	auto pos = Part->GetRelativeLocation();
	Part->SetRelativeLocation(FVector(pos.X, pos.Y, max));
}

void USinglePartFixer::ProcessRequest(float value)
{
	AwaitingRequestCount--;
	heights.Add(value);
	if (AwaitingRequestCount == 0)
	{
		FixBuilding();
	}
}

void UBuildingElevationFixer::FixBuildings()
{
	for (auto building : Buildings)
	{
		for (auto part : building->Parts)
		{
			auto fixer = USinglePartFixer::Init(part, ElevationProvider);
		}
	}
}
