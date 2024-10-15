#ifndef CAM_CUTDATABASE_H
#define CAM_CUTDATABASE_H

class CAM_CutDataBase
{
public:
    enum class CutType
    {
        Unknown = 0,
        CutData,
        DiffCutData
    };
public:
    CutType getCutType() { return cutType; }
protected:
    CutType cutType;
};

#endif // CAM_CUTDATABASE_H
