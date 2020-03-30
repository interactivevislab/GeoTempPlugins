#include "PostgisLoader.h"
#include "CoreMinimal.h"
#if !NOPOSTGRES
#include "libpq-fe.h"
#endif
void APostgisReader::CheckConnectionStatus(float DeltaTime)
{
#if NOPOSTGRES
	throw "Postgresql was not installed on build pc";
#else
	if (conn != NULL) {
		auto status = PQstatus(conn);

		if (status == CONNECTION_BAD) {
			FString s(PQerrorMessage(conn));
			UE_LOG(LogTemp, Warning, TEXT("Postgress: Connection to database failed: %s"), *s);
			if (!s.IsEmpty()) {
				Error = FString::Format(TEXT("Postgress: Connection to database failed: %s"), { s });
				//if (conn) PQfinish(conn);
				bool result;
				InitConnect(result);
			}			
			Status = EStatus::Unconnected;
		}
		else {
			if (status == CONNECTION_OK) {
				UE_LOG(LogTemp, Display, TEXT("Postgress: Connection Complete."));
				Status = EStatus::Connected;
				return;
			}
			auto pollResult = PQconnectPoll(conn);
			if (pollResult == PGRES_POLLING_FAILED) {
				auto e = PQerrorMessage(conn);
				FString s(e);
				UE_LOG(LogTemp, Warning, TEXT("Postgress: Connection to database failed(poll): %s"), *s);
				Error = FString::Format(TEXT("Postgress: Connection to database failed(poll): %s"), { s });
				//if (conn) PQfinish(conn);
				Status = EStatus::Unconnected;
			}
			else if (pollResult != PGRES_POLLING_OK) {
				UE_LOG(LogTemp, Display, TEXT("Postgress: Awaiting connection (poll)."));
			} else {
				/*switch (status)
				{
				case CONNECTION_STARTED: UE_LOG(LogTemp, Message, TEXT("Connection started.")); break;
				case CONNECTION_MADE:  UE_LOG(LogTemp, Message, TEXT("Connection made.")); break;
				case CONNECTION_AWAITING_RESPONSE:  UE_LOG(LogTemp, Message, TEXT("Awaiting connection.")); break;
				case CONNECTION_AUTH_OK:  UE_LOG(LogTemp, Message, TEXT("Auth OK.")); break;
				case CONNECTION_SETENV:  UE_LOG(LogTemp, Message, TEXT("Connection started.")); break;
				case CONNECTION_SSL_STARTUP:UE_LOG(LogTemp, Message, TEXT("SSL Startup.")); break;
				case CONNECTION_NEEDED: UE_LOG(LogTemp, Message, TEXT("Need connection.")); break;
				case CONNECTION_CHECK_WRITABLE: UE_LOG(LogTemp, Message, TEXT("Check writable.")); break;
				case CONNECTION_CONSUME: UE_LOG(LogTemp, Message, TEXT("Connection Consume.")); break;
				case CONNECTION_OK:UE_LOG(LogTemp, Message, TEXT("Connection Complete.")); Status = Connected; break;
				}*/
			}
		}
	}
	else 
	{
		if (Status != EStatus::AwaitingConnection) {
			bool result;
			InitConnect(result);
		}
	}
#endif
}

TArray<FPostGisBinaryEntity> APostgisReader::ExecuteRawQuery(FString query, int geometryColumnIndex)
{
#if NOPOSTGRES
		throw "Postgresql is not installed on build pc"; return TArray<FPostGisBinaryEntity>();
#else
	CheckConnectionStatus(0);
	if (Status != EStatus::Connected || conn == nullptr)
	{
		Error = TEXT("Attempt to use database before connection estabilished");
		return TArray<FPostGisBinaryEntity>();
	}

	PGresult* result;
	result = PQexecParams(conn, TCHAR_TO_ANSI(*query), 0, NULL, NULL, NULL, NULL, 1);

	if (result != nullptr) {
		auto v = PQresultStatus(result);
		if (v != PGRES_TUPLES_OK)
		{
			FString e = FString(PQresultErrorMessage(result));
			UE_LOG(LogTemp, Warning, TEXT("%s"), *e);
			Error = e;
			return TArray<FPostGisBinaryEntity>();
		}
	} else
	{		
		FString e = FString(PQerrorMessage(conn));
		UE_LOG(LogTemp, Warning, TEXT("%s"), *e);
		Error = e;
		return TArray<FPostGisBinaryEntity>();
	}
	TArray<FPostGisBinaryEntity> ret;
	int count = PQntuples(result);
	int fc = PQnfields(result);
	for (int i = 0; i < count; i++)
	{
		try
		{
			FPostGisBinaryEntity retValue;
			if (geometryColumnIndex >= 0) {
				auto byteCount = PQgetlength(result, i, geometryColumnIndex);
				if (byteCount == 0) continue;
				auto c = PQgetvalue(result, i, geometryColumnIndex) + 1;
				retValue.Geometry.Append(reinterpret_cast<const unsigned char*>(c), byteCount);
			}			
			for (int j = 0; j < fc; j++)
			{
				if (j == geometryColumnIndex) continue;
				retValue.Tags.Add(FString(PQfname(result, j)), FString(UTF8_TO_TCHAR(PQgetvalue(result, i, j))));
			}
			ret.Add(retValue);
		}
		catch (FString e)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s"), *e);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Query sucessfully executed, returned %i elements"), count);
	return ret;
#endif
}

TMap<FString, FPostGisBinaryEntity> APostgisReader::ExecuteIndexedRawQuery(FString query, int keyColumnIndex, int geometryColumnIndex)
{
#if NOPOSTGRES
	throw "Postgresql is not installed on build pc"; return TArray<FPostGisBinaryEntity>();
#else
	CheckConnectionStatus(0);
	if (Status != EStatus::Connected || conn == nullptr)
	{
		Error = TEXT("Attempt to use database before connection estabilished");
		return TMap<FString, FPostGisBinaryEntity>();
	}

	PGresult* result;
	result = PQexecParams(conn, TCHAR_TO_ANSI(*query), 0, NULL, NULL, NULL, NULL, 1);

	if (result != nullptr) {
		auto v = PQresultStatus(result);
		if (v != PGRES_TUPLES_OK)
		{
			FString e = FString(PQresultErrorMessage(result));
			UE_LOG(LogTemp, Warning, TEXT("%s"), *e);
			Error = e;
			return TMap<FString, FPostGisBinaryEntity>();
		}
	}
	else
	{
		FString e = FString(PQerrorMessage(conn));
		UE_LOG(LogTemp, Warning, TEXT("%s"), *e);
		Error = e;
		return TMap<FString, FPostGisBinaryEntity>();
	}
	TMap<FString, FPostGisBinaryEntity> ret;
	int count = PQntuples(result);
	int fc = PQnfields(result);
	for (int i = 0; i < count; i++)
	{
		try
		{
			FPostGisBinaryEntity retValue;
			if (geometryColumnIndex >= 0) {
				auto byteCount = PQgetlength(result, i, geometryColumnIndex);
				if (byteCount == 0) continue;
				auto c = PQgetvalue(result, i, geometryColumnIndex) + 1;
				retValue.Geometry.Append(reinterpret_cast<const unsigned char*>(c), byteCount);
			}
			FString key;
			for (int j = 0; j < fc; j++)
			{
				if (j == geometryColumnIndex) continue;
				if (j == keyColumnIndex)
				{
					key = FString(PQgetvalue(result, i, j));
				}
				retValue.Tags.Add(FString(PQfname(result, j)), FString(UTF8_TO_TCHAR(PQgetvalue(result, i, j))));
			}
			ret.Add(key, retValue);
		}
		catch (FString e)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s"), *e);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Query sucessfully executed, returned %i elements"), count);
	return ret;
#endif
}

void APostgisReader::InitConnect(bool& sucess)
{
#if NOPOSTGRES
	throw "Postgresql is not installed on build pc";
#else
		
	auto s = FString::Format(TEXT("host={0} port={1} dbname={2} user={3} password={4}"), { Host, Port, DatabaseName, User, Pass });
	const char* conninfo = TCHAR_TO_ANSI(*s);

	/* Make a connection to the database */
	conn = PQconnectStart(conninfo);

	if (conn == NULL) {
		UE_LOG(LogTemp, Warning, TEXT("Postgress: Connection to database was not initialized"));
		sucess = false;
	}
	else {
		IsInit = true;
		sucess = true;
		Status = EStatus::AwaitingConnection;
		UE_LOG(LogTemp, Display, TEXT("Postgress: Connection to database sucessfully initialized. Awaiting response..."));
	}
#endif
}


void APostgisReader::BeginPlay()
{
	Super::BeginPlay();
}


void APostgisReader::BeginDestroy()
{
	//if (conn) PQfinish(conn);
	AActor::BeginDestroy();
}


void APostgisReader::LoadConfiguration() {
	LoadConfig();
	ReloadConfig();
}


void APostgisReader::SaveConfiguration() {
	SaveConfig();
}


FPosgisContourData APostgisReader::CreateContourFromBinary(FPostGisBinaryEntity entity,
	ProjectionType projection, float originLon,
	float originLat)
{	
	auto byteCount = entity.Geometry.Num();
	auto c = entity.Geometry.GetData();
	FPosgisContourData contour;
	contour.ZeroLat = originLat;
	contour.ZeroLon = originLon;
	auto typeVal = ((uint32*)c)[0];
	int offset = 4;

	if (typeVal >> 12 != 0)
	{
		throw TEXT("3+d geometry is not yet supported");
	}
	float height = 0;

	switch (typeVal)
	{
		case 1:
		{
			//point
			FVector* point = contour.BinaryParsePoint(c, offset, projection, height);
		}
		break;
		case 2:
		{
			//line
			contour.BinaryParseCurve(c, offset, projection, false, height);
		}
		break;
		case 3:
		{
			//polygon
			auto other = contour.BinaryParsePolygon(c, offset, projection, false, height);
			contour.Append(other);
		}
		break;
		case 4:
		{
			//multi-point
			uint32 pcount = *((uint32*)(c + offset * sizeof(char)));
			offset += 4;
			for (uint32 pi = 0; pi < pcount; pi++)
			{
				FVector* point = contour.BinaryParsePoint(c, offset, projection, height);
			}
		}
		break;
		case 5:
		{
			//multi-line
			uint32 pcount = *((uint32*)(c + offset * sizeof(char)));
			offset += 4;
			for (uint32 pi = 0; pi < pcount; pi++)
			{
				contour.BinaryParseCurve(c, offset, projection, true, height);
			}
		}
		break;
		case 6:
		{
			//multi-polygon
			uint32 polyCount = *((uint32*)(c + offset * sizeof(char)));
			offset += 4;

			std::vector<std::vector<float>>* contours = new std::vector<std::vector<float>>;
			for (uint32 pi = 0; pi < polyCount; pi++)
			{
				auto cont = contour.BinaryParsePolygon(c, offset, projection, true, height);
				contour.Append(cont);
			}
		}
	}
	contour.Tags = entity.Tags;
	return contour;
}



FPosgisLinesData APostgisReader::CreateCurvesFromBinary(FPostGisBinaryEntity entity,
	ProjectionType projection, float originLon,
	float originLat)
{
	FPosgisLinesData ret;	
	auto byteCount = entity.Geometry.Num();
	auto c = entity.Geometry.GetData();
	FPosgisContourData contour;
	contour.ZeroLat = originLat;
	contour.ZeroLon = originLon;
	auto typeVal = ((uint32*)c)[0];
	int offset = 4;

	if (typeVal >> 12 != 0)
	{
		throw TEXT("3+d geometry is not yet supported");
	}
	float height = 0;

	switch (typeVal)
	{
	case 1:
	{
		//point
		FVector* point = contour.BinaryParsePoint(c, offset, projection, height);
	}
	break;
	case 2:
	{
		//line
		auto points = contour.BinaryParseCurve(c, offset, projection, false, height);
		ret.lines.Add(FContour(points));
	}
	break;
	case 3:
	{
		//polygon
		auto contData = contour.BinaryParsePolygon(c, offset, projection, false, height);
		for (auto cont : contData->Outer)
		{
			ret.lines.Add(cont);
		}
		for (auto cont : contData->Holes)
		{
			ret.lines.Add(cont);
		}
	}
	break;
	case 4:
	{
		//multi-point
		uint32 pcount = *((uint32*)(c + offset * sizeof(char)));
		offset += 4;
		for (uint32 pi = 0; pi < pcount; pi++)
		{
			FVector* point = contour.BinaryParsePoint(c, offset, projection, height);
		}
	}
	break;
	case 5:
	{
		//multi-line
		uint32 pcount = *((uint32*)(c + offset * sizeof(char)));
		offset += 4;
		for (uint32 pi = 0; pi < pcount; pi++)
		{
			auto curve = contour.BinaryParseCurve(c, offset, projection, true, height);
			ret.lines.Add(FContour(curve));
		}
	}
	break;
	case 6:
	{
		//multi-polygon
		uint32 polyCount = *((uint32*)(c + offset * sizeof(char)));
		offset += 4;

		std::vector<std::vector<float>>* contours = new std::vector<std::vector<float>>;
		for (uint32 pi = 0; pi < polyCount; pi++)
		{
			auto poly = contour.BinaryParsePolygon(c, offset, projection, true, height);
			for (auto cont : poly->Outer)
			{				
				ret.lines.Add(FContour(cont));
			}
			for (auto cont : poly->Holes)
			{
				ret.lines.Add(FContour(cont));
			}
		}
	}
	}	
	return ret;
}
