#pragma once

#include <QString>
#include <QMap>
/*!
 *
 */
namespace Mode
{
	const int Wait = 0;
	const int Measure = 1;
	const int Save = 2;
	const int Calc = 3;

	const QMap<int,QString> str = {
		{Wait, QString("Wait")},
		{Measure, QString("Measure")},
		{Save, QString("Save")},
		{Calc, QString("Calc")}
	};
}
