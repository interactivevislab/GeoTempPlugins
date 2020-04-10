#include "PostgisReader.h"

#if !NOPOSTGRES
#include "libpq-fe.h"
#endif


void APostgisReader::CheckConnectionStatus(float inDeltaTime)
{
#if NOPOSTGRES
	throw "Postgresql was not installed on build pc";
#else

	if (conn == nullptr)
	{
		if (Status != EStatus::AwaitingConnection)
		{
			bool result;
			InitConnect(result);
		}
		return;
	}

	auto status = PQstatus(conn);

	switch (status)
	{
		case CONNECTION_BAD:
		{
			FString badConErrMsg(PQerrorMessage(conn));
			UE_LOG(LogTemp, Warning, TEXT("Postgress: Connection to database failed: %s"), *badConErrMsg);
			if (!badConErrMsg.IsEmpty())
			{
				Error = FString::Format(TEXT("Postgress: Connection to database failed: %s"), { badConErrMsg });
				//if (conn) PQfinish(conn);
				bool result;
				InitConnect(result);
			}
			Status = EStatus::Unconnected;
			break;
		}
		
		case CONNECTION_OK:
		{
			UE_LOG(LogTemp, Display, TEXT("Postgress: Connection Complete."));
			Status = EStatus::Connected;
			break;
		}
		
		default:
		{
			auto pollResult = PQconnectPoll(conn);
			if (pollResult == PGRES_POLLING_FAILED)
			{
				auto e = PQerrorMessage(conn);
				FString errorMsg(e);
				UE_LOG(LogTemp, Warning, TEXT("Postgress: Connection to database failed(poll): %s"), *errorMsg);
				Error = FString::Format(TEXT("Postgress: Connection to database failed(poll): %s"), { errorMsg });
				//if (conn) PQfinish(conn);
				Status = EStatus::Unconnected;
			}
			else if (pollResult != PGRES_POLLING_OK)
			{
				UE_LOG(LogTemp, Display, TEXT("Postgress: Awaiting connection (poll)."));
			}
			else
			{
				/*switch (status)
				{
					case CONNECTION_STARTED:			UE_LOG(LogTemp, Message, TEXT("Connection started."));	break;
					case CONNECTION_MADE:				UE_LOG(LogTemp, Message, TEXT("Connection made."));		break;
					case CONNECTION_AWAITING_RESPONSE:	UE_LOG(LogTemp, Message, TEXT("Awaiting connection."));	break;
					case CONNECTION_AUTH_OK:			UE_LOG(LogTemp, Message, TEXT("Auth OK."));				break;
					case CONNECTION_SETENV:				UE_LOG(LogTemp, Message, TEXT("Connection started."));	break;
					case CONNECTION_SSL_STARTUP:		UE_LOG(LogTemp, Message, TEXT("SSL Startup."));			break;
					case CONNECTION_NEEDED:				UE_LOG(LogTemp, Message, TEXT("Need connection."));		break;
					case CONNECTION_CHECK_WRITABLE:		UE_LOG(LogTemp, Message, TEXT("Check writable."));		break;
					case CONNECTION_CONSUME:			UE_LOG(LogTemp, Message, TEXT("Connection Consume."));	break;
					case CONNECTION_OK:					UE_LOG(LogTemp, Message, TEXT("Connection Complete.")); Status = Connected; break;
				}*/
			}
			break;
		}
	}
#endif
}


TArray<FWkbEntity> APostgisReader::ExecuteRawQuery(FString inQuery, int inGeometryColumnIndex)
{
#if NOPOSTGRES
		throw "Postgresql is not installed on build pc";
		return TArray<FWkbEntity>();
#else

	CheckConnectionStatus(0);
	if (Status != EStatus::Connected || conn == nullptr)
	{
		Error = TEXT("Attempt to use database before connection estabilished");
		return TArray<FWkbEntity>();
	}

	PGresult* result;
	result = PQexecParams(conn, TCHAR_TO_ANSI(*inQuery), 0, nullptr, nullptr, nullptr, nullptr, 1);

	if (result != nullptr)
	{
		auto v = PQresultStatus(result);
		if (v != PGRES_TUPLES_OK)
		{
			FString e = FString(PQresultErrorMessage(result));
			UE_LOG(LogTemp, Warning, TEXT("%s"), *e);
			Error = e;
			return TArray<FWkbEntity>();
		}
	}
	else
	{		
		FString e = FString(PQerrorMessage(conn));
		UE_LOG(LogTemp, Warning, TEXT("%s"), *e);
		Error = e;
		return TArray<FWkbEntity>();
	}

	TArray<FWkbEntity> ret;
	int count = PQntuples(result);
	int fc = PQnfields(result);
	for (int i = 0; i < count; i++)
	{
		try
		{
			FWkbEntity retValue;
			if (inGeometryColumnIndex >= 0)
			{
				auto byteCount = PQgetlength(result, i, inGeometryColumnIndex);
				if (byteCount == 0)
				{
					continue;
				}
				auto c = PQgetvalue(result, i, inGeometryColumnIndex) + 1;
				retValue.Geometry.Append(reinterpret_cast<const unsigned char*>(c), byteCount);
			}			
			for (int j = 0; j < fc; j++)
			{
				if (j == inGeometryColumnIndex)
				{
					continue;
				}
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


TMap<FString, FWkbEntity> APostgisReader::ExecuteIndexedRawQuery(FString inQuery,
	int inKeyColumnIndex, int inGeometryColumnIndex)
{
#if NOPOSTGRES
	throw "Postgresql is not installed on build pc";
	return TMap<FString, FWkbEntity>();
#else

	CheckConnectionStatus(0);
	if (Status != EStatus::Connected || conn == nullptr)
	{
		Error = TEXT("Attempt to use database before connection estabilished");
		return TMap<FString, FWkbEntity>();
	}

	PGresult* result;
	result = PQexecParams(conn, TCHAR_TO_ANSI(*inQuery), 0, nullptr, nullptr, nullptr, nullptr, 1);

	if (result != nullptr)
	{
		auto v = PQresultStatus(result);
		if (v != PGRES_TUPLES_OK)
		{
			FString e = FString(PQresultErrorMessage(result));
			UE_LOG(LogTemp, Warning, TEXT("%s"), *e);
			Error = e;
			return TMap<FString, FWkbEntity>();
		}
	}
	else
	{
		FString e = FString(PQerrorMessage(conn));
		UE_LOG(LogTemp, Warning, TEXT("%s"), *e);
		Error = e;
		return TMap<FString, FWkbEntity>();
	}

	TMap<FString, FWkbEntity> ret;
	int count = PQntuples(result);
	int fc = PQnfields(result);
	for (int i = 0; i < count; i++)
	{
		try
		{
			FWkbEntity retValue;
			if (inGeometryColumnIndex >= 0)
			{
				auto byteCount = PQgetlength(result, i, inGeometryColumnIndex);
				if (byteCount == 0)
				{ 
					continue;
				}
				auto c = PQgetvalue(result, i, inGeometryColumnIndex) + 1;
				retValue.Geometry.Append(reinterpret_cast<const unsigned char*>(c), byteCount);
			}
			FString key;
			for (int j = 0; j < fc; j++)
			{
				if (j == inGeometryColumnIndex)
				{
					continue;
				}
				if (j == inKeyColumnIndex)
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


void APostgisReader::InitConnect(bool& outSuccess)
{
#if NOPOSTGRES
	throw "Postgresql is not installed on build pc";
#else
		
	auto s = FString::Format(	TEXT("host={0}	port={1}	dbname={2}		user={3}	password={4}"), 
									{ Host,		Port,		DatabaseName,	User,		Pass });
	const char* conninfo = TCHAR_TO_ANSI(*s);

	/* Make a connection to the database */
	conn = PQconnectStart(conninfo);

	if (conn == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Postgress: Connection to database was not initialized"));
		outSuccess = false;
	}
	else
	{
		IsInit = true;
		outSuccess = true;
		Status = EStatus::AwaitingConnection;
		UE_LOG(LogTemp, Display, TEXT("Postgress: Connection to database sucessfully initialized. Awaiting response..."));
	}
#endif
}


void APostgisReader::LoadConfiguration()
{
	LoadConfig();
	ReloadConfig();
}


void APostgisReader::SaveConfiguration()
{
	SaveConfig();
}


FContourData APostgisReader::CreateContourFromBinary(FWkbEntity inEntity, FGeoCoords inGeoCoords)
{	
	auto byteCount = inEntity.Geometry.Num();
	auto c = inEntity.Geometry.GetData();
	FContourData contour;
	contour.ZeroLat = inGeoCoords.ZeroLat;
	contour.ZeroLon = inGeoCoords.ZeroLon;
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
			FVector point = contour.BinaryParsePoint(c, offset, inGeoCoords, height);
		}
		break;

		case 2:
		{
			//line
			contour.BinaryParseCurve(c, offset, inGeoCoords, false, height);
		}
		break;

		case 3:
		{
			//polygon
			auto other = contour.BinaryParsePolygon(c, offset, inGeoCoords, false, height);
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
				FVector point = contour.BinaryParsePoint(c, offset, inGeoCoords, height);
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
				contour.BinaryParseCurve(c, offset, inGeoCoords, true, height);
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
				auto cont = contour.BinaryParsePolygon(c, offset, inGeoCoords, true, height);
				contour.Append(cont);
			}
		}
		break;
	}

	contour.Tags = inEntity.Tags;
	return contour;
}


FLinesData APostgisReader::CreateCurvesFromBinary(FWkbEntity inEntity, FGeoCoords inGeoCoords)
{
	FLinesData ret;	
	auto byteCount = inEntity.Geometry.Num();
	auto c = inEntity.Geometry.GetData();
	FContourData contour;
	contour.ZeroLat = inGeoCoords.ZeroLat;
	contour.ZeroLon = inGeoCoords.ZeroLon;
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
			FVector point = contour.BinaryParsePoint(c, offset, inGeoCoords, height);
		}
		break;

		case 2:
		{
			//line
			auto points = contour.BinaryParseCurve(c, offset, inGeoCoords, false, height);
			ret.Lines.Add(FContour(points));
		}
		break;

		case 3:
		{
			//polygon
			auto contData = contour.BinaryParsePolygon(c, offset, inGeoCoords, false, height);
			for (auto cont : contData.Outer)
			{
				ret.Lines.Add(cont);
			}
			for (auto cont : contData.Holes)
			{
				ret.Lines.Add(cont);
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
				FVector point = contour.BinaryParsePoint(c, offset, inGeoCoords, height);
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
				auto curve = contour.BinaryParseCurve(c, offset, inGeoCoords, true, height);
				ret.Lines.Add(FContour(curve));
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
				auto poly = contour.BinaryParsePolygon(c, offset, inGeoCoords, true, height);
				for (auto cont : poly.Outer)
				{
					ret.Lines.Add(FContour(cont));
				}
				for (auto cont : poly.Holes)
				{
					ret.Lines.Add(FContour(cont));
				}
			}
		}
		break;
	}

	return ret;
}
