#include "cam_gencode.h"
#include "cam_cutdata.h"
#include "cam_diffcutdata.h"

CAM_GenCode::CAM_GenCode() {}

QString CAM_GenCode::generateGCode(QList<CAM_CutDataBase *> dataArr, QList<RS_Vector> startPts)
{
    QStringList mainSb;
    QStringList subSb;

    RS_Vector curPt;

    for (int i = 0; i < dataArr.count(); i++) {
        if (curPt.valid) {
            mainSb.append("# Move to next path...\n");
            mainSb.append("G91;\n");
            mainSb.append(QString("G00X%1Y%2;\n")
                  .arg(toFixed(round(startPts[i].x) - round(curPt.x)),
                       toFixed(round(startPts[i].y) - round(curPt.y))));
            mainSb.append("\n");
        }
        curPt = startPts[i];

        CAM_CutDataBase *data = dataArr[i];
        if (data->getCutType() == CAM_CutDataBase::CutType::CutData) {
            CAM_CutData *cutData = (CAM_CutData *)data;
            if (cutData->getGenPathConfig().useAbsCommand) {
                mainSb.append(genOneCutCodeAbs(*cutData));
            }
            else {
                mainSb.append(genOneCutCodeRelMainPart(*cutData, i * 2 + 1));
                subSb.append(genOneCutCodeRelSubPart(*cutData, i * 2 + 1));
            }
        }
        else if (data->getCutType() == CAM_CutDataBase::CutType::DiffCutData) {
            CAM_DiffCutData *diffCutData = (CAM_DiffCutData *)data;
            if (diffCutData->getMainPathCutData().getGenPathConfig().useAbsCommand) {
                mainSb.append(genDiffCutCodeAbs(*diffCutData));
            }
            else {
                mainSb.append(genDiffCutCodeRelMainPart(*diffCutData, i * 2 + 1));
                subSb.append(genDiffCutCodeRelSubPart(*diffCutData, i * 2 + 1));
            }
        }

    }

    QStringList sb;
    sb.append("%\n");
    sb.append("\n");

    sb.append(mainSb);

    sb.append("M02;\n");

    sb.append(subSb);

    sb.append("\n");
    sb.append("%\n");
    sb.append("\n");

    return sb.join("");
}

QString CAM_GenCode::generateOneCutCode(const CAM_CutData &cutdata)
{
    QList<CAM_CutDataBase *> dataArr;
    dataArr.append((CAM_CutDataBase *)&cutdata);
    QList<RS_Vector> startPts;
    startPts.append(RS_Vector(0, 0));
    return generateGCode(dataArr, startPts);
}

QString CAM_GenCode::generateDiffCutCode(const CAM_DiffCutData &diffCutData)
{
    QList<CAM_CutDataBase *> dataArr;
    dataArr.append((CAM_CutDataBase *)&diffCutData);
    QList<RS_Vector> startPts;
    startPts.append(RS_Vector(0, 0));
    return generateGCode(dataArr, startPts);
}

double CAM_GenCode::round(double num)
{
    return ::round(num * 1000) / 1000;
}

QString CAM_GenCode::toFixed(double num)
{
    QString result = QString::number(num, 'f', 3);
    if (!result.contains('.')) {
        result += ".";
    }
    if (result.startsWith("-.")) {
        result = result.replace("-.", "-0.");
    }
    else if (result.startsWith(".")) {
        result = result.replace(".", "0.");
    }
    return result;
}

QStringList CAM_GenCode::genOneCutCodeAbs(const CAM_CutData &data)
{
    QStringList sb;
    bool useMacro = data.getGenPathConfig().paramUseMacro;
    if (useMacro) {
        for (int i = 0; i < data.getGenPathConfig().cutTime; i++) {
            sb.append(QString("`ELEC_PAR%1;\n").arg(i + 1));
        }
    }
    sb.append("\n");

    if (useMacro) {
        sb.append(QString("E`ELEC_CUT%1;\n").arg(0 + 1));
    }
    else {
        sb.append(QString("E%1;\n").arg(data.getGenPathConfig().elecData[0]));
    }

    sb.append("G90;\n");
    sb.append("G92X0.Y0.;\n");
    sb.append("M17;\n");

    RS_Vector curPt = RS_Vector(0, 0);

    curPt += data.getPathData().cutOffPt - data.getPathData().threadPt;
    sb.append(QString("G01X%1Y%2;\n")
                  .arg(toFixed(round(curPt.x)),
                       toFixed(round(curPt.y))));
    sb.append("M01;\n");
    sb.append("\n");

    for (int i = 0; i < data.getGenPathConfig().cutTime; i++) {
        // 送电参数
        if (useMacro) {
            sb.append(QString("E`ELEC_CUT%1;\n").arg(i + 1));
        }
        else {
            sb.append(QString("E%1;\n").arg(data.getGenPathConfig().elecData[i]));
        }

        // 切入段
        if (i % 2 == 0) {
            curPt += data.getPathData().startPt - data.getPathData().cutOffPt;
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(round(curPt.x)),
                               toFixed(round(curPt.y))));
        }
        else {
            curPt += data.getPathData().endPt - data.getPathData().exitPt;
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(round(curPt.x)),
                               toFixed(round(curPt.y))));
        }

        // 刀补
        double compDist = data.getGenPathConfig().compData[i];
        bool compFlag = true;
        if (i % 2 != 0) {
            // 反走，反转偏移
            compDist = -compDist;
            compFlag = !compFlag;
        }
        if (!data.getPathData().compensateDir) {
            compDist = -compDist;
            compFlag = !compFlag;
        }
        if (useMacro) {
            if (compFlag) {
                sb.append(QString("G41D`COMP_CUT%1;\n").arg(i + 1));
            }
            else {
                sb.append(QString("G42D`COMP_CUT%1;\n").arg(i + 1));
            }
        }
        else {
            if (compDist != 0) {
                if (compDist > 0) {
                    // 左偏
                    sb.append(QString("G41D%1;\n").arg(toFixed(compDist)));
                }
                else {
                    // 右偏
                    sb.append(QString("G42D%1;\n").arg(toFixed(-compDist)));
                }
            }
        }

        // 锥补
        if (data.getGenPathConfig().tapeEnabled) {
            double tapeAngle = data.getGenPathConfig().tapeAngle;
            if (i % 2 != 0) {
                tapeAngle = -tapeAngle;
            }

            if (tapeAngle < 0) {
                sb.append(QString("G52T%1;\n").arg(toFixed(-tapeAngle)));
            }
            else {
                sb.append(QString("G51T%1;\n").arg(toFixed(tapeAngle)));
            }
        }

        // 主路径
        if (i % 2 == 0) {
            for (int j = 0; j < data.getPathData().mainPath.count(); j++) {
                CAM_Segment seg = data.getPathData().mainPath[j];
                RS_Vector dist = seg.getEndPt() - seg.getStartPt();
                curPt += dist;

                if (seg.getBulge() == 0) {
                    sb.append(QString("G01X%1Y%2;\n")
                                  .arg(toFixed(curPt.x), toFixed(curPt.y)));
                }
                else {
                    RS_Vector center = utils2d.circleCenter(seg);
                    double iDist = round(center.x) - round(seg.getStartX());
                    double jDist = round(center.y) - round(seg.getStartY());

                    if (seg.getBulge() < 0) {
                        // 顺时针
                        sb.append(QString("G02X%1Y%2I%3J%4;\n")
                                      .arg(toFixed(curPt.x),
                                           toFixed(curPt.y),
                                           toFixed(iDist),
                                           toFixed(jDist)));
                    }
                    else {
                        sb.append(QString("G03X%1Y%2I%3J%4;\n")
                                      .arg(toFixed(curPt.x),
                                          toFixed(curPt.y),
                                          toFixed(iDist),
                                          toFixed(jDist)));
                    }
                }
            }
        }
        else {
            for (int j = 0; j < data.getPathData().mainPath.count(); j++) {
                CAM_Segment invSeg = data.getPathData().mainPath[data.getPathData().mainPath.count() - 1 - j];

                CAM_Segment seg = CAM_Segment(invSeg.getEndPt(), invSeg.getStartPt(), -invSeg.getBulge());
                RS_Vector dist = seg.getEndPt() - seg.getStartPt();
                curPt += dist;

                if (seg.getBulge() == 0) {
                    sb.append(QString("G01X%1Y%2;\n")
                                  .arg(toFixed(curPt.x),
                                       toFixed(curPt.y)));
                }
                else {
                    RS_Vector center = utils2d.circleCenter(seg);
                    double iDist = round(center.x) - round(seg.getStartX());
                    double jDist = round(center.y) - round(seg.getStartY());

                    if (seg.getBulge() < 0) {
                        // 顺时针
                        sb.append(QString("G02X%1Y%2I%3J%4;\n")
                                      .arg(toFixed(curPt.x),
                                           toFixed(curPt.y),
                                           toFixed(iDist),
                                           toFixed(jDist)));
                    }
                    else {
                        sb.append(QString("G03X%1Y%2I%3J%4;\n")
                                      .arg(toFixed(curPt.x),
                                           toFixed(curPt.y),
                                           toFixed(iDist),
                                           toFixed(jDist)));
                    }
                }
            }
        }

        // 关刀补
        if (useMacro) {
            sb.append("G40;\n");
        }
        else {
            if (compDist != 0) {
                sb.append("G40;\n");
            }
        }

        // 关锥补
        if (data.getGenPathConfig().tapeEnabled) {
            sb.append("G50;\n");
        }

        // 退出段
        if (i % 2 == 0) {
            curPt += data.getPathData().exitPt - data.getPathData().endPt;
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(round(curPt.x)),
                               toFixed(round(curPt.y))));
        }
        else {
            curPt += data.getPathData().cutOffPt - data.getPathData().startPt;
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(round(curPt.x)),
                               toFixed(round(curPt.y))));
        }

        sb.append("M01;\n");
        sb.append("\n");
    }

    if (data.getPathData().isClosed) {
        // 切落段
        if (useMacro) {
            sb.append(QString("E`ELEC_CUT%1").arg(0 + 1));
        }
        else {
            sb.append(QString("E%1").arg(data.getGenPathConfig().elecData[0]));
        }

        if (data.getGenPathConfig().cutTime == 1) {
            // 奇数次切割，从终点切起点
            curPt += data.getPathData().cutOffPt - data.getPathData().exitPt;
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(round(curPt.x)),
                               toFixed(round(curPt.y))));
            sb.append("M18;\n");
        }
        else {
            // 偶数次切割，从起点切终点，然后空走返回
            curPt += data.getPathData().exitPt - data.getPathData().cutOffPt;
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(round(curPt.x)),
                               toFixed(round(curPt.y))));
            sb.append("M18;\n");
            curPt += data.getPathData().cutOffPt - data.getPathData().exitPt;
            sb.append(QString("G00X%1Y%2;\n")
                          .arg(toFixed(round(curPt.x)),
                               toFixed(round(curPt.y))));
        }
    }
    else {
        sb.append("M18;\n");
    }

    // 回穿丝点
    if (data.getPathData().isClosed) {
        curPt += data.getPathData().threadPt - data.getPathData().cutOffPt;
        sb.append(QString("G00X%1Y%2;\n")
                      .arg(toFixed(round(curPt.x)),
                           toFixed(round(curPt.y))));
    }

    sb.append("\n");
    return sb;
}

QStringList CAM_GenCode::genOneCutCodeRelMainPart(const CAM_CutData &data, int startSubIndex)
{
    QStringList sb;
    bool useMacro = data.getGenPathConfig().paramUseMacro;

    if (useMacro) {
        for (int i = 0; i < data.getGenPathConfig().cutTime; i++) {
            sb.append(QString("`ELEC_PAR%1;\n").arg(i + 1));
        }
    }
    sb.append("\n");

    if (useMacro) {
        sb.append(QString("E`ELEC_CUT%1;\n").arg(0 + 1));
    }
    else {
        sb.append(QString("E%1;\n").arg(data.getGenPathConfig().elecData[0]));
    }

    sb.append("G91;\n");
    sb.append("M17;\n");

    sb.append(QString("G01X%1Y%2;\n")
                  .arg(toFixed(round(data.getPathData().cutOffPt.x) - round(data.getPathData().threadPt.x)),
                       toFixed(round(data.getPathData().cutOffPt.y) - round(data.getPathData().threadPt.y))));
    sb.append("M01;\n");
    sb.append("\n");

    for (int i = 0; i < data.getGenPathConfig().cutTime; i++) {
        // 送电参数
        if (useMacro) {
            sb.append(QString("E`ELEC_CUT%1;\n").arg(i + 1));
        }
        else {
            sb.append(QString("E%1;\n").arg(data.getGenPathConfig().elecData[i]));
        }

        // 切入段
        if (i % 2 == 0) {
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(round(data.getPathData().startPt.x) - round(data.getPathData().cutOffPt.x)),
                               toFixed(round(data.getPathData().startPt.y) - round(data.getPathData().cutOffPt.y))));
        }
        else {
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(round(data.getPathData().endPt.x) - round(data.getPathData().exitPt.x)),
                               toFixed(round(data.getPathData().endPt.y) - round(data.getPathData().exitPt.y))));
        }

        // 刀补
        double compDist = data.getGenPathConfig().compData[i];
        bool compFlag = true;
        if (i % 2 != 0) {
            // 反走，反转偏移
            compDist = -compDist;
            compFlag = !compFlag;
        }
        if (!data.getPathData().compensateDir) {
            compDist = -compDist;
            compFlag = !compFlag;
        }
        if (useMacro) {
            if (compFlag) {
                sb.append(QString("G41D`COMP_CUT%1;\n").arg(i + 1));
            }
            else {
                sb.append(QString("G42D`COMP_CUT%1;\n").arg(i + 1));
            }
        }
        else {
            if (compDist != 0) {
                if (compDist > 0) {
                    // 左偏
                    sb.append(QString("G41D%1;\n").arg(toFixed(compDist)));
                }
                else {
                    // 右偏
                    sb.append(QString("G42D%1;\n").arg(toFixed(-compDist)));
                }
            }
        }

        // 锥补
        if (data.getGenPathConfig().tapeEnabled) {
            double tapeAngle = data.getGenPathConfig().tapeAngle;
            if (i % 2 != 0) {
                tapeAngle = -tapeAngle;
            }

            if (tapeAngle < 0) {
                sb.append(QString("G52T%1;\n").arg(toFixed(-tapeAngle)));
            }
            else {
                sb.append(QString("G51T%1;\n").arg(toFixed(tapeAngle)));
            }
        }

        if (i % 2 == 0) {
            sb.append(QString("M98P%1;\n").arg(startSubIndex));
        }
        else {
            sb.append(QString("M98P%1;\n").arg(startSubIndex + 1));
        }

        // 关刀补
        if (useMacro) {
            sb.append("G40;\n");
        }
        else {
            if (compDist != 0) {
                sb.append("G40;\n");
            }
        }

        // 关锥补
        if (data.getGenPathConfig().tapeEnabled) {
            sb.append("G50;\n");
        }

        // 退出段
        if (i % 2 == 0) {
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(round(data.getPathData().exitPt.x) - round(data.getPathData().endPt.x)),
                               toFixed(round(data.getPathData().exitPt.y) - round(data.getPathData().endPt.y))));
        }
        else {
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(round(data.getPathData().cutOffPt.x) - round(data.getPathData().startPt.x)),
                               toFixed(round(data.getPathData().cutOffPt.y) - round(data.getPathData().startPt.y))));
        }

        sb.append("M01;\n");
        sb.append("\n");
    }

    if (data.getPathData().isClosed) {
        // 切落段
        if (useMacro) {
            sb.append(QString("E`ELEC_CUT%1;\n").arg(0 + 1));
        }
        else {
            sb.append(QString("E%1;\n").arg(data.getGenPathConfig().elecData[0]));
        }

        if (data.getGenPathConfig().cutTime % 2 == 1) {
            // 奇数次切割，从终点切起点
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(round(data.getPathData().cutOffPt.x) - round(data.getPathData().exitPt.x)),
                               toFixed(round(data.getPathData().cutOffPt.y) - round(data.getPathData().exitPt.y))));
            sb.append("M18;\n");
        }
        else {
            // 偶数次切割，从起点切终点，然后空走返回
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(round(data.getPathData().exitPt.x) - round(data.getPathData().cutOffPt.x)),
                               toFixed(round(data.getPathData().exitPt.y) - round(data.getPathData().cutOffPt.y))));
            sb.append("M18;\n");
            sb.append(QString("G00X%1Y%2;\n")
                          .arg(toFixed(round(data.getPathData().cutOffPt.x) - round(data.getPathData().exitPt.x)),
                               toFixed(round(data.getPathData().cutOffPt.y) - round(data.getPathData().exitPt.y))));
        }
    }
    else {
        sb.append("M18;\n");
    }

    // 回穿丝点
    if (data.getPathData().isClosed) {
        sb.append(QString("G00X%1Y%2;\n")
                      .arg(toFixed(round(data.getPathData().threadPt.x) - round(data.getPathData().cutOffPt.x)),
                           toFixed(round(data.getPathData().threadPt.y) - round(data.getPathData().cutOffPt.y))));
    }

    sb.append("\n");
    return sb;
}

QStringList CAM_GenCode::genOneCutCodeRelSubPart(const CAM_CutData &data, int startSubIndex)
{
    QStringList sb;
    // 正向路径
    sb.append("\n");
    sb.append(QString("N%1;\n").arg(startSubIndex));

    for (int i = 0; i < data.getPathData().mainPath.count(); i++) {
        CAM_Segment seg = data.getPathData().mainPath[i];
        double xDist = seg.getEndX() - seg.getStartX();
        double yDist = seg.getEndY() - seg.getStartY();
        if (seg.getBulge() == 0) {
            sb.append(QString("G01X%1Y%2;\n")
                          .arg(toFixed(xDist),
                               toFixed(yDist)));
        }
        else {
            RS_Vector center = utils2d.circleCenter(seg);
            double iDist = round(center.x) - round(seg.getStartX());
            double jDist = round(center.y) - round(seg.getStartY());
            if (seg.getBulge() < 0) {
                sb.append(QString("G02X%1Y%2I%3J%4;\n")
                              .arg(toFixed(xDist),
                                   toFixed(yDist),
                                   toFixed(iDist),
                                   toFixed(jDist)));
            }
            else {
                sb.append(QString("G03X%1Y%2I%3J%4;\n")
                              .arg(toFixed(xDist),
                                   toFixed(yDist),
                                   toFixed(iDist),
                                   toFixed(jDist)));
            }
        }
    }

    sb.append("M99;\n");

    if (data.getGenPathConfig().cutTime > 1) {
        // 反向路径
        sb.append("\n");
        sb.append(QString("N%1;\n").arg(startSubIndex + 1));
        for (int i = 0; i < data.getPathData().mainPath.count(); i++) {
            CAM_Segment invSeg = data.getPathData().mainPath[data.getPathData().mainPath.count() - 1 - i];
            CAM_Segment seg = CAM_Segment(invSeg.getEndPt(), invSeg.getStartPt(), -invSeg.getBulge());
            double xDist = seg.getEndX() - seg.getStartX();
            double yDist = seg.getEndY() - seg.getStartY();
            if (seg.getBulge() == 0) {
                sb.append(QString("G01X%1Y%2;\n")
                              .arg(toFixed(xDist),
                                   toFixed(yDist)));
            }
            else {
                RS_Vector center = utils2d.circleCenter(seg);
                double iDist = round(center.x) - round(seg.getStartX());
                double jDist = round(center.y) - round(seg.getStartY());
                if (seg.getBulge() < 0) {
                    sb.append(QString("G02X%1Y%2I%3J%4;\n")
                                  .arg(toFixed(xDist),
                                       toFixed(yDist),
                                       toFixed(iDist),
                                       toFixed(jDist)));
                }
                else {
                    sb.append(QString("G03X%1Y%2I%3J%4;\n")
                                  .arg(toFixed(xDist),
                                       toFixed(yDist),
                                       toFixed(iDist),
                                       toFixed(jDist)));
                }
            }
        }

        sb.append("M99;\n");
    }

    return sb;
}

QStringList CAM_GenCode::genDiffCutCodeAbs(const CAM_DiffCutData &data)
{
    QStringList sb;
    bool useMacro = data.getMainPathCutData().getGenPathConfig().paramUseMacro;

    if (useMacro) {
        for (int i = 0; i < data.getMainPathCutData().getGenPathConfig().cutTime; i++) {
            sb.append(QString("`ELEC_PAR%1;\n").arg(i + 1));
        }
    }
    sb.append("\n");

    if (useMacro) {
        sb.append(QString("E`ELEC_CUT%1;\n").arg(0 + 1));
    }
    else {
        sb.append(QString("E%1;\n").arg(data.getMainPathCutData().getGenPathConfig().elecData[0]));
    }

    sb.append("G90;\n");
    sb.append("G92X0.Y0.;\n");
    sb.append("M17;\n");

    RS_Vector curXY = RS_Vector(0, 0);
    RS_Vector curUV = RS_Vector(0, 0);

    curXY += data.getMainPathCutData().getPathData().cutOffPt - data.getMainPathCutData().getPathData().threadPt;
    curUV += data.getSubPathCutData().getPathData().cutOffPt - data.getSubPathCutData().getPathData().threadPt;
    sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                  .arg(toFixed(round(curXY.x)),
                       toFixed(round(curXY.y)),
                       toFixed(round(curUV.x)),
                       toFixed(round(curUV.y))));
    sb.append("M01;\n");
    sb.append("\n");

    for (int i = 0; i < data.getMainPathCutData().getGenPathConfig().cutTime; i++) {
        // 送电参数
        if (useMacro) {
            sb.append(QString("E`ELEC_CUT%1;\n").arg(i + 1));
        }
        else {
            sb.append(QString("E%1;\n").arg(data.getMainPathCutData().getGenPathConfig().elecData[i]));
        }

        // 切入段
        if (i % 2 == 0) {
            curXY += data.getMainPathCutData().getPathData().startPt - data.getMainPathCutData().getPathData().cutOffPt;
            curUV += data.getSubPathCutData().getPathData().startPt - data.getSubPathCutData().getPathData().cutOffPt;
            sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                          .arg(toFixed(round(curXY.x)),
                               toFixed(round(curXY.y)),
                               toFixed(round(curUV.x)),
                               toFixed(round(curUV.y))));
        }
        else {
            curXY += data.getMainPathCutData().getPathData().endPt - data.getMainPathCutData().getPathData().exitPt;
            curUV += data.getSubPathCutData().getPathData().endPt - data.getSubPathCutData().getPathData().exitPt;
            sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                          .arg(toFixed(round(curXY.x)),
                               toFixed(round(curXY.y)),
                               toFixed(round(curUV.x)),
                               toFixed(round(curUV.y))));
        }

        if (i % 2 == 0) {
            // 奇数次切割，从起始点到终点
            for (int j = 0; j < data.getMainPathCutData().getPathData().mainPath.count(); j++) {
                CAM_Segment seg = data.getMainPathCutData().getPathData().mainPath[j];
                RS_Vector dist = seg.getEndPt() - seg.getStartPt();
                curXY += dist;
                if (seg.getBulge() == 0) {
                    sb.append(QString("G01X%1Y%2")
                                  .arg(toFixed(round(curXY.x)),
                                       toFixed(round(curXY.y))));
                }
                else {
                    RS_Vector center = utils2d.circleCenter(seg);
                    double iDist = round(center.x) - round(seg.getStartX());
                    double jDist = round(center.y) - round(seg.getStartY());
                    if (seg.getBulge() < 0) {
                        sb.append(QString("G02X%1Y%2I%3J%4")
                                      .arg(toFixed(round(curXY.x)),
                                           toFixed(round(curXY.y)),
                                           toFixed(iDist),
                                           toFixed(jDist)));
                    }
                    else {
                        sb.append(QString("G03X%1Y%2I%3J%4")
                                      .arg(toFixed(round(curXY.x)),
                                           toFixed(round(curXY.y)),
                                           toFixed(iDist),
                                           toFixed(jDist)));
                    }
                }

                sb.append(":");

                seg = data.getSubPathCutData().getPathData().mainPath[j];
                dist = seg.getEndPt() - seg.getStartPt();
                curUV += dist;
                if (seg.getBulge() == 0) {
                    sb.append(QString("G01X%1Y%2")
                                  .arg(toFixed(round(curUV.x)),
                                       toFixed(round(curUV.y))));
                }
                else {
                    RS_Vector center = utils2d.circleCenter(seg);
                    double iDist = round(center.x) - round(seg.getStartX());
                    double jDist = round(center.y) - round(seg.getStartY());
                    if (seg.getBulge() < 0) {
                        sb.append(QString("G02X%1Y%2I%3J%4")
                                      .arg(toFixed(round(curUV.x)),
                                           toFixed(round(curUV.y)),
                                           toFixed(iDist),
                                           toFixed(jDist)));
                    }
                    else {
                        sb.append(QString("G03X%1Y%2I%3J%4")
                                      .arg(toFixed(round(curUV.x)),
                                           toFixed(round(curUV.y)),
                                           toFixed(iDist),
                                           toFixed(jDist)));
                    }
                }

                sb.append(";\n");
            }
        }
        else {
            for (int j = 0; j < data.getMainPathCutData().getPathData().mainPath.count(); j++) {
                CAM_Segment invSeg = data.getMainPathCutData().getPathData().mainPath[data.getMainPathCutData().getPathData().mainPath.count() - 1 - j];
                CAM_Segment seg = CAM_Segment(invSeg.getEndPt(), invSeg.getStartPt(), -invSeg.getBulge());
                RS_Vector dist = seg.getEndPt() - seg.getStartPt();
                curXY += dist;
                if (seg.getBulge() == 0) {
                    sb.append(QString("G01X%1Y%2")
                                  .arg(toFixed(round(curXY.x)),
                                       toFixed(round(curXY.y))));
                }
                else {
                    RS_Vector center = utils2d.circleCenter(seg);
                    double iDist = round(center.x) - round(seg.getStartX());
                    double jDist = round(center.y) - round(seg.getStartY());
                    if (seg.getBulge() < 0) {
                        sb.append(QString("G02X%1Y%2I%3J%4")
                                      .arg(toFixed(round(curXY.x)),
                                           toFixed(round(curXY.y)),
                                           toFixed(iDist),
                                           toFixed(jDist)));
                    }
                    else {
                        sb.append(QString("G03X%1Y%2I%3J%4")
                                      .arg(toFixed(round(curXY.x)),
                                           toFixed(round(curXY.y)),
                                           toFixed(iDist),
                                           toFixed(jDist)));
                    }
                }

                sb.append(":");

                invSeg = data.getSubPathCutData().getPathData().mainPath[data.getSubPathCutData().getPathData().mainPath.count() - 1 - j];
                seg = CAM_Segment(invSeg.getEndPt(), invSeg.getStartPt(), -invSeg.getBulge());
                dist = seg.getEndPt() - seg.getStartPt();
                curUV += dist;
                if (seg.getBulge() == 0) {
                    sb.append(QString("G01X%1Y%2")
                                  .arg(toFixed(round(curUV.x)),
                                       toFixed(round(curUV.y))));
                }
                else {
                    RS_Vector center = utils2d.circleCenter(seg);
                    double iDist = round(center.x) - round(seg.getStartX());
                    double jDist = round(center.y) - round(seg.getStartY());
                    if (seg.getBulge() < 0) {
                        sb.append(QString("G02X%1Y%2I%3J%4")
                                      .arg(toFixed(round(curUV.x)),
                                           toFixed(round(curUV.y)),
                                           toFixed(iDist),
                                           toFixed(jDist)));
                    }
                    else {
                        sb.append(QString("G03X%1Y%2I%3J%4")
                                      .arg(toFixed(round(curUV.x)),
                                           toFixed(round(curUV.y)),
                                           toFixed(iDist),
                                           toFixed(jDist)));
                    }
                }

                sb.append(";\n");
            }
        }

        // 退出段
        if (i % 2 == 0) {
            curXY += data.getMainPathCutData().getPathData().exitPt - data.getMainPathCutData().getPathData().endPt;
            curUV += data.getSubPathCutData().getPathData().exitPt - data.getSubPathCutData().getPathData().endPt;
            sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                          .arg(toFixed(round(curXY.x)),
                               toFixed(round(curXY.y)),
                               toFixed(round(curUV.x)),
                               toFixed(round(curUV.y))));
        }
        else {
            curXY += data.getMainPathCutData().getPathData().cutOffPt - data.getMainPathCutData().getPathData().startPt;
            curUV += data.getSubPathCutData().getPathData().cutOffPt - data.getSubPathCutData().getPathData().startPt;
            sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                          .arg(toFixed(round(curXY.x)),
                               toFixed(round(curXY.y)),
                               toFixed(round(curUV.x)),
                               toFixed(round(curUV.y))));
        }

        sb.append("M01;\n");
        sb.append("\n");
    }

    if (data.getMainPathCutData().getPathData().isClosed) {
        // 切落段
        if (useMacro) {
            sb.append(QString("E`ELEC_CUT%1").arg(0 + 1));
        }
        else {
            sb.append(QString("E%1").arg(data.getMainPathCutData().getGenPathConfig().elecData[0]));
        }

        if (data.getMainPathCutData().getGenPathConfig().cutTime % 2 == 1) {
            // 奇数次切割，从终点切起点
            curXY += data.getMainPathCutData().getPathData().cutOffPt - data.getMainPathCutData().getPathData().exitPt;
            curUV += data.getSubPathCutData().getPathData().cutOffPt - data.getSubPathCutData().getPathData().exitPt;
            sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                          .arg(toFixed(round(curXY.x)),
                               toFixed(round(curXY.y)),
                               toFixed(round(curUV.x)),
                               toFixed(round(curUV.y))));
            sb.append("M18;\n");
        }
        else {
            // 偶数次切割，从起点切终点，然后空走返回
            curXY += data.getMainPathCutData().getPathData().exitPt - data.getMainPathCutData().getPathData().cutOffPt;
            curUV += data.getSubPathCutData().getPathData().exitPt - data.getSubPathCutData().getPathData().cutOffPt;
            sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                          .arg(toFixed(round(curXY.x)),
                               toFixed(round(curXY.y)),
                               toFixed(round(curUV.x)),
                               toFixed(round(curUV.y))));
            sb.append("M18;\n");
            curXY += data.getMainPathCutData().getPathData().cutOffPt - data.getMainPathCutData().getPathData().exitPt;
            curUV += data.getSubPathCutData().getPathData().cutOffPt - data.getSubPathCutData().getPathData().exitPt;
            sb.append(QString("G00X%1Y%2:G00X%3Y%4;\n")
                          .arg(toFixed(round(curXY.x)),
                               toFixed(round(curXY.y)),
                               toFixed(round(curUV.x)),
                               toFixed(round(curUV.y))));
        }
    }
    else {
        sb.append("M18;\n");
    }

    // 回穿丝点
    if (data.getMainPathCutData().getPathData().isClosed) {
        curXY += data.getMainPathCutData().getPathData().threadPt - data.getMainPathCutData().getPathData().cutOffPt;
        curUV += data.getSubPathCutData().getPathData().threadPt - data.getSubPathCutData().getPathData().cutOffPt;
        sb.append(QString("G00X%1Y%2:G00X%3Y%4;\n")
                      .arg(toFixed(round(curXY.x)),
                           toFixed(round(curXY.y)),
                           toFixed(round(curUV.x)),
                           toFixed(round(curUV.y))));
    }

    sb.append("\n");
    return sb;
}

QStringList CAM_GenCode::genDiffCutCodeRelMainPart(const CAM_DiffCutData &data, int startSubIndex)
{
    QStringList sb;
    bool useMacro = data.getMainPathCutData().getGenPathConfig().paramUseMacro;

    if (useMacro) {
        for (int i = 0; i < data.getMainPathCutData().getGenPathConfig().cutTime; i++) {
            sb.append(QString("`ELEC_PAR%1;\n").arg(i + 1));
        }
    }
    sb.append("\n");

    if (useMacro) {
        sb.append(QString("E`ELEC_CUT%1;\n").arg(0 + 1));
    }
    else {
        sb.append(QString("E%1;\n").arg(data.getMainPathCutData().getGenPathConfig().elecData[0]));
    }

    sb.append("G91;\n");
    sb.append("M17;\n");

    sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                  .arg(toFixed(round(data.getMainPathCutData().getPathData().cutOffPt.x) - round(data.getMainPathCutData().getPathData().threadPt.x)),
                       toFixed(round(data.getMainPathCutData().getPathData().cutOffPt.y) - round(data.getMainPathCutData().getPathData().threadPt.y)),
                       toFixed(round(data.getSubPathCutData().getPathData().cutOffPt.x) - round(data.getSubPathCutData().getPathData().threadPt.x)),
                       toFixed(round(data.getSubPathCutData().getPathData().cutOffPt.y) - round(data.getSubPathCutData().getPathData().threadPt.y))));
    sb.append("M01;\n");
    sb.append("\n");

    for (int i = 0; i < data.getMainPathCutData().getGenPathConfig().cutTime; i++) {
        // 送电参数
        if (useMacro) {
            sb.append(QString("E`ELEC_CUT%1;\n").arg(i + 1));
        }
        else {
            sb.append(QString("E%1;\n").arg(data.getMainPathCutData().getGenPathConfig().elecData[i]));
        }

        // 切入段
        if (i % 2 == 0) {
            sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                          .arg(toFixed(round(data.getMainPathCutData().getPathData().startPt.x) - round(data.getMainPathCutData().getPathData().cutOffPt.x)),
                               toFixed(round(data.getMainPathCutData().getPathData().startPt.y) - round(data.getMainPathCutData().getPathData().cutOffPt.y)),
                               toFixed(round(data.getSubPathCutData().getPathData().startPt.x) - round(data.getSubPathCutData().getPathData().cutOffPt.x)),
                               toFixed(round(data.getSubPathCutData().getPathData().startPt.y) - round(data.getSubPathCutData().getPathData().cutOffPt.y))));
        }
        else {
            sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                          .arg(toFixed(round(data.getMainPathCutData().getPathData().endPt.x) - round(data.getMainPathCutData().getPathData().exitPt.x)),
                               toFixed(round(data.getMainPathCutData().getPathData().endPt.y) - round(data.getMainPathCutData().getPathData().exitPt.y)),
                               toFixed(round(data.getSubPathCutData().getPathData().endPt.x) - round(data.getSubPathCutData().getPathData().exitPt.x)),
                               toFixed(round(data.getSubPathCutData().getPathData().endPt.y) - round(data.getSubPathCutData().getPathData().exitPt.y))));
        }

        if (i % 2 == 0) {
            // 奇数次切割，从起始点到终点
            sb.append(QString("M98P%1;\n").arg(startSubIndex));
        }
        else {
            // 偶数次切割，从终点到起点
            sb.append(QString("M98P%1;\n").arg(startSubIndex + 1));
        }

        // 退出段
        if (i % 2 == 0) {
            sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                          .arg(toFixed(round(data.getMainPathCutData().getPathData().exitPt.x) - round(data.getMainPathCutData().getPathData().endPt.x)),
                               toFixed(round(data.getMainPathCutData().getPathData().exitPt.y) - round(data.getMainPathCutData().getPathData().endPt.y)),
                               toFixed(round(data.getSubPathCutData().getPathData().exitPt.x) - round(data.getSubPathCutData().getPathData().endPt.x)),
                               toFixed(round(data.getSubPathCutData().getPathData().exitPt.y) - round(data.getSubPathCutData().getPathData().endPt.y))));
        }
        else {
            sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                          .arg(toFixed(round(data.getMainPathCutData().getPathData().cutOffPt.x) - round(data.getMainPathCutData().getPathData().startPt.x)),
                               toFixed(round(data.getMainPathCutData().getPathData().cutOffPt.y) - round(data.getMainPathCutData().getPathData().startPt.y)),
                               toFixed(round(data.getSubPathCutData().getPathData().cutOffPt.x) - round(data.getSubPathCutData().getPathData().startPt.x)),
                               toFixed(round(data.getSubPathCutData().getPathData().cutOffPt.y) - round(data.getSubPathCutData().getPathData().startPt.y))));
        }

        sb.append("M01;\n");
        sb.append("\n");
    }

    if (data.getMainPathCutData().getPathData().isClosed) {
        // 切落段
        if (useMacro) {
            sb.append(QString("E`ELEC_CUT%1").arg(0 + 1));
        }
        else {
            sb.append(QString("E%1").arg(data.getMainPathCutData().getGenPathConfig().elecData[0]));
        }

        if (data.getMainPathCutData().getGenPathConfig().cutTime % 2 == 1) {
            // 奇数次切割，从终点切起点
            sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                          .arg(toFixed(round(data.getMainPathCutData().getPathData().cutOffPt.x) - round(data.getMainPathCutData().getPathData().exitPt.x)),
                               toFixed(round(data.getMainPathCutData().getPathData().cutOffPt.y) - round(data.getMainPathCutData().getPathData().exitPt.y)),
                               toFixed(round(data.getSubPathCutData().getPathData().cutOffPt.x) - round(data.getSubPathCutData().getPathData().exitPt.x)),
                               toFixed(round(data.getSubPathCutData().getPathData().cutOffPt.y) - round(data.getSubPathCutData().getPathData().exitPt.y))));
            sb.append("M18;\n");
        }
        else {
            // 偶数次切割，从起点切终点，然后空走返回
            sb.append(QString("G01X%1Y%2:G01X%3Y%4;\n")
                          .arg(toFixed(round(data.getMainPathCutData().getPathData().exitPt.x) - round(data.getMainPathCutData().getPathData().cutOffPt.x)),
                               toFixed(round(data.getMainPathCutData().getPathData().exitPt.y) - round(data.getMainPathCutData().getPathData().cutOffPt.y)),
                               toFixed(round(data.getSubPathCutData().getPathData().exitPt.x) - round(data.getSubPathCutData().getPathData().cutOffPt.x)),
                               toFixed(round(data.getSubPathCutData().getPathData().exitPt.y) - round(data.getSubPathCutData().getPathData().cutOffPt.y))));
            sb.append("M18;\n");
            sb.append(QString("G00X%1Y%2:G00X%3Y%4;\n")
                          .arg(toFixed(round(data.getMainPathCutData().getPathData().cutOffPt.x) - round(data.getMainPathCutData().getPathData().exitPt.x)),
                               toFixed(round(data.getMainPathCutData().getPathData().cutOffPt.y) - round(data.getMainPathCutData().getPathData().exitPt.y)),
                               toFixed(round(data.getSubPathCutData().getPathData().cutOffPt.x) - round(data.getSubPathCutData().getPathData().exitPt.x)),
                               toFixed(round(data.getSubPathCutData().getPathData().cutOffPt.y) - round(data.getSubPathCutData().getPathData().exitPt.y))));
        }
    }
    else {
        sb.append("M18;\n");
    }

    // 回穿丝点
    if (data.getMainPathCutData().getPathData().isClosed) {
        sb.append(QString("G00X%1Y%2:G00X%3Y%4;\n")
                      .arg(toFixed(round(data.getMainPathCutData().getPathData().threadPt.x) - round(data.getMainPathCutData().getPathData().cutOffPt.x)),
                           toFixed(round(data.getMainPathCutData().getPathData().threadPt.y) - round(data.getMainPathCutData().getPathData().cutOffPt.y)),
                           toFixed(round(data.getSubPathCutData().getPathData().threadPt.x) - round(data.getSubPathCutData().getPathData().cutOffPt.x)),
                           toFixed(round(data.getSubPathCutData().getPathData().threadPt.y) - round(data.getSubPathCutData().getPathData().cutOffPt.y))));
    }

    sb.append("\n");
    return sb;
}

QStringList CAM_GenCode::genDiffCutCodeRelSubPart(const CAM_DiffCutData &data, int startSubIndex)
{
    QStringList sb;
    // 正向路径
    sb.append("\n");
    sb.append(QString("N%1;\n").arg(startSubIndex));

    for (int i = 0; i < data.getMainPathCutData().getPathData().mainPath.count(); i++) {
        CAM_Segment seg = data.getMainPathCutData().getPathData().mainPath[i];
        double xDist = seg.getEndX() - seg.getStartX();
        double yDist = seg.getEndY() - seg.getStartY();
        if (seg.getBulge() == 0) {
            sb.append(QString("G01X%1Y%2")
                          .arg(toFixed(xDist),
                               toFixed(yDist)));
        }
        else {
            RS_Vector center = utils2d.circleCenter(seg);
            double iDist = round(center.x) - round(seg.getStartX());
            double jDist = round(center.y) - round(seg.getStartY());
            if (seg.getBulge() < 0) {
                sb.append(QString("G02X%1Y%2I%3J%4")
                              .arg(toFixed(xDist),
                                   toFixed(yDist),
                                   toFixed(iDist),
                                   toFixed(jDist)));
            }
            else {
                sb.append(QString("G03X%1Y%2I%3J%4")
                              .arg(toFixed(xDist),
                                   toFixed(yDist),
                                   toFixed(iDist),
                                   toFixed(jDist)));
            }
        }
        sb.append(":");

        seg = data.getSubPathCutData().getPathData().mainPath[i];
        double uDist = seg.getEndX() - seg.getStartX();
        double vDist = seg.getEndY() - seg.getStartY();
        if (seg.getBulge() == 0) {
            sb.append(QString("G01X%1Y%2")
                          .arg(toFixed(uDist),
                               toFixed(vDist)));
        }
        else {
            RS_Vector center = utils2d.circleCenter(seg);
            double iDist = round(center.x) - round(seg.getStartX());
            double jDist = round(center.y) - round(seg.getStartY());
            if (seg.getBulge() < 0) {
                sb.append(QString("G02X%1Y%2I%3J%4")
                              .arg(toFixed(uDist),
                                   toFixed(vDist),
                                   toFixed(iDist),
                                   toFixed(jDist)));
            }
        }

        sb.append(";\n");
    }

    sb.append("M99;\n");

    if (data.getMainPathCutData().getGenPathConfig().cutTime > 1) {
        // 反向路径
        sb.append("\n");
        sb.append(QString("N%1;\n").arg(startSubIndex + 1));
        for (int i = 0; i < data.getMainPathCutData().getPathData().mainPath.count(); i++) {
            CAM_Segment invSeg = data.getMainPathCutData().getPathData().mainPath[data.getMainPathCutData().getPathData().mainPath.count() - 1 - i];
            CAM_Segment seg = CAM_Segment(invSeg.getEndPt(), invSeg.getStartPt(), -invSeg.getBulge());
            double xDist = seg.getEndX() - seg.getStartX();
            double yDist = seg.getEndY() - seg.getStartY();
            if (seg.getBulge() == 0) {
                sb.append(QString("G01X%1Y%2")
                              .arg(toFixed(xDist),
                                   toFixed(yDist)));
            }
            else {
                RS_Vector center = utils2d.circleCenter(seg);
                double iDist = round(center.x) - round(seg.getStartX());
                double jDist = round(center.y) - round(seg.getStartY());
                if (seg.getBulge() < 0) {
                    sb.append(QString("G02X%1Y%2I%3J%4")
                                  .arg(toFixed(xDist),
                                       toFixed(yDist),
                                       toFixed(iDist),
                                       toFixed(jDist)));
                }
                else {
                    sb.append(QString("G03X%1Y%2I%3J%4")
                                  .arg(toFixed(xDist),
                                       toFixed(yDist),
                                       toFixed(iDist),
                                       toFixed(jDist)));
                }
            }
            sb.append(":");

            invSeg = data.getSubPathCutData().getPathData().mainPath[data.getSubPathCutData().getPathData().mainPath.count() - 1 - i];
            seg = CAM_Segment(invSeg.getEndPt(), invSeg.getStartPt(), -invSeg.getBulge());
            double uDist = seg.getEndX() - seg.getStartX();
            double vDist = seg.getEndY() - seg.getStartY();
            if (seg.getBulge() == 0) {
                sb.append(QString("G01X%1Y%2")
                              .arg(toFixed(uDist),
                                   toFixed(vDist)));
            }
            else {
                RS_Vector center = utils2d.circleCenter(seg);
                double iDist = round(center.x) - round(seg.getStartX());
                double jDist = round(center.y) - round(seg.getStartY());
                if (seg.getBulge() < 0) {
                    sb.append(QString("G02X%1Y%2I%3J%4")
                                  .arg(toFixed(uDist),
                                       toFixed(vDist),
                                       toFixed(iDist),
                                       toFixed(jDist)));
                }
                else {
                    sb.append(QString("G03X%1Y%2I%3J%4")
                                  .arg(toFixed(uDist),
                                       toFixed(vDist),
                                       toFixed(iDist),
                                       toFixed(jDist)));
                }
            }

            sb.append(";\n");
        }

        sb.append("M99;\n");
    }

    return sb;
}























