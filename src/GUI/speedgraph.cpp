#include <QLocale>
#include "data/data.h"
#include "tooltip.h"
#include "format.h"
#include "speedgraphitem.h"
#include "speedgraph.h"


SpeedGraph::SpeedGraph(QWidget *parent) : GraphTab(parent)
{
	_units = Metric;
	_timeType = Total;
	_showTracks = true;

	setYUnits();
	setYLabel(tr("Speed"));

	setSliderPrecision(1);
}

void SpeedGraph::setInfo()
{
	if (_showTracks) {
		QLocale l(QLocale::system());
		QString pace = Format::timeSpan((3600.0 / (avg() * yScale())), false);
		QString pu = (_units == Metric) ? tr("min/km") : (_units == Imperial) ?
		  tr("min/mi") : tr("min/nmi");

		GraphView::addInfo(tr("Average"), l.toString(avg() * yScale(), 'f',
		  1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), l.toString(max() * yScale(), 'f',
		  1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Pace"), pace + UNIT_SPACE + pu);
	} else
		clearInfo();
}

QList<GraphItem*> SpeedGraph::loadData(const Data &data)
{
	QList<GraphItem*> graphs;

	for (int i = 0; i < data.tracks().count(); i++) {
		const Track &track = data.tracks().at(i);
		const Graph &graph = track.speed();

		if (!graph.isValid()) {
			skipColor();
			graphs.append(0);
		} else {
			SpeedGraphItem *gi = new SpeedGraphItem(graph, _graphType,
			  track.movingTime());
			gi->setTimeType(_timeType);
			GraphView::addGraph(gi);
			_avg.append(QPointF(track.distance(), gi->avg()));
			_mavg.append(QPointF(track.distance(), gi->mavg()));
			graphs.append(gi);
		}
	}

	for (int i = 0; i < data.routes().count(); i++) {
		skipColor();
		graphs.append(0);
	}

	for (int i = 0; i < data.areas().count(); i++)
		skipColor();

	setInfo();
	redraw();

	return graphs;
}

qreal SpeedGraph::avg() const
{
	qreal sum = 0, w = 0;
	const QVector<QPointF> &vector = (_timeType == Moving) ? _mavg : _avg;

	for (int i = 0; i < vector.size(); i++) {
		const QPointF &p = vector.at(i);
		sum += p.y() * p.x();
		w += p.x();
	}

	return (sum / w);
}

void SpeedGraph::clear()
{
	_avg.clear();
	_mavg.clear();

	GraphTab::clear();
}

void SpeedGraph::setYUnits()
{
	if (_units == Nautical) {
		GraphView::setYUnits(tr("kn"));
		setYScale(MS2KN);
	} else if (_units == Imperial) {
		GraphView::setYUnits(tr("mi/h"));
		setYScale(MS2MIH);
	} else {
		GraphView::setYUnits(tr("km/h"));
		setYScale(MS2KMH);
	}
}

void SpeedGraph::setUnits(Units units)
{
	_units = units;

	setYUnits();
	setInfo();

	GraphView::setUnits(units);
}

void SpeedGraph::setTimeType(enum TimeType type)
{
	_timeType = type;

	for (int i = 0; i < _graphs.size(); i++)
		static_cast<SpeedGraphItem*>(_graphs.at(i))->setTimeType(type);

	setInfo();
	redraw();
}

void SpeedGraph::showTracks(bool show)
{
	_showTracks = show;

	showGraph(show);
	setInfo();

	redraw();
}
