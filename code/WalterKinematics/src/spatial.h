/*
 * spatial.h
 *
 * Data structures liked points, vectors and matrixes
 *
 *  Created on: 07.08.2016
 *      Author: JochenAlt
 */

#ifndef SPATIAL_H_
#define SPATIAL_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#include "matrix/cmatrix"
#pragma GCC diagnostic pop

#include "Setup.h"
#include "Util.h"

using techsoft::mslice;
typedef techsoft::matrix<rational>  Matrix;
typedef techsoft::matrix<rational>  HomMatrix;
typedef std::valarray<rational> HomVector;
typedef std::valarray<rational> Vector;


// POSE_LINEAR interpolates in a linear manner between two tcp poses,
// POSE_CUBIC_BEZIER does the same but with a bezier curve
// JOINT_LINEAR interpolates in a linear manner but on base of angles (not on tcp's position)
enum InterpolationType { POSE_LINEAR, POSE_CUBIC_BEZIER, JOINT_LINEAR};	// trajectories are built with these types of interpolation


class Point {
	 friend ostream& operator<<(ostream&, const Point&);
	public:
		Point();
		Point(const Point& p);
		Point(const HomVector& p) {
			x = p[X];
			y = p[Y];
			z = p[Z];
		}

		Point(rational xP,rational yP, rational zP);
		void translate(const Point& pPoint);
		void mirrorAt(const Point& pPoint, rational scale);
		void mirrorAt(const Point& pPoint);
		void set(rational pX, rational pY,rational pZ);
		void null();
		bool isNull();


		void operator= (const Point& p) {
			x = p.x;
			y = p.y;
			z = p.z;
		}

		void operator= (const HomVector& p) {
			x = p[X];
			y = p[Y];
			z = p[Z];
		}

		void operator+= (const Point& p) {
			x += p.x;
			y += p.y;
			z += p.z;
		}

		void operator-= (const Point& p) {
			x -= p.x;
			y -= p.y;
			z -= p.z;
		}

		void operator*= (const rational f) {
			x *= f;
			y *= f;
			z *= f;
		}

		void operator/= (const rational f) {
			float xrf= 1.0/f;
			x *= xrf;
			y *= xrf;
			z *= xrf;
		}

		Point operator- (const Point& p) const{
			Point result= (*this);
			result -= p;
			return result;
		}

		Point operator+ (const Point& p) const{
			Point result= (*this);
			result += p;
			return result;
		}

		Point operator/ (const rational f) const{
			Point result= (*this);
			result*= (1./f);
			return result;
		}

		Point operator* (const rational f) const{
			Point result= (*this);
			result*=f;
			return result;
		}


		bool operator==(const Point& pos) {
			return almostEqual(x,pos.x, floatPrecision) && almostEqual(y, pos.y, floatPrecision) && almostEqual(z, pos.z, floatPrecision);
		};

		bool operator!=(const Point& pos) {
			return !((*this) == pos);
		};


		rational& operator[] (int idx)  {
			switch (idx) {
				case X:	return x;break;
				case Y:	return y;break;
				case Z:	return z;break;
				default:
				break;
			}
			return x;
		};

		rational operator[] (int idx)  const {
			switch (idx) {
				case X:	return x;break;
				case Y:	return y;break;
				case Z:	return z;break;
				default:
				break;
			}
			return x;
		};

		rational distance(const Point& p) const;
		rational length() const;
		rational angleToDegree(const Point& pPoint) const;
		rational scalarProduct(const Point& pPoint) const;
		Point orthogonalProjection(const Point& pLine) const;
		Point orthogonalProjection(const Point& pLineA, const Point &pLineB) const;
		Point getPointOfLine(rational ratio, const Point& target);

		// returns the homogenous vector, i.e. a 4-dimensional vector with 1.0 as last dimension
		HomVector getHomVector() const {
			HomVector result = { x,y,z,1.0 };
			return result;
		}

		string toString() const;
		bool fromString(const string& str, int &idx);

		rational x;
		rational y;
		rational z;
};


class Rotation : public Point {
	 friend ostream& operator<<(ostream&, const Rotation&);
	public:
		Rotation () : Point (0,0,0){};
		Rotation(float xP,float yP, float zP): Point(xP,yP,zP) {
			x = xP;
			y = yP;
			z = zP;
		}

		Rotation(const Rotation& p) : Point(p) {
			x= p.x;
			y= p.y;
			z= p.z;
		};
		void operator=(const Rotation& rot) {
			x= rot.x;
			y= rot.y;
			z= rot.z;
		};
		void operator+=(const Rotation& rot) {
			x += rot.x;
			y += rot.y;
			z += rot.z;
		};

		void operator*=(float f) {
			x *= f;
			y *= f;
			z *= f;
		};
		void operator/=(float f) {
			x /= f;
			y /= f;
			z /= f;
		};
		Rotation operator*(const float f) const {
			Rotation result(*this);
			result.x *= f;
			result.y *= f;
			result.z *= f;
			return result;
		};
		Rotation operator/(const float f) const {
			Rotation result(*this);
			result *= (1./f);
			return result;
		};

		Rotation operator+(const Rotation& rot) const {
			Rotation result(*this);
			result += rot;
			return result;
		};

		Rotation operator-(const Rotation& rot) const {
			Rotation result(*this);
			result -= rot;
			return result;
		};
		bool operator==(const Rotation& pos) {
			return (x == pos.x) && (y == pos.y) && (z == pos.z);
		};

		bool operator!=(const Rotation& pos) {
			return !((*this) == pos);
		};

		string toString() const;
		bool fromString(const string& str, int &idx);
};

class JointAngles {
public:
	 friend ostream& operator<<(ostream&, const JointAngles&);

	JointAngles() {
		null();
	}

	void setDefaultPosition() {
		a[0] = 0.0;
		a[1] = 0.0;
		a[2] = 0.0;
		a[3] = 0.0;
		a[4] = 0.0;
		a[5] = 0.0;
		a[6] = radians(35.0);
	}

	static JointAngles getDefaultPosition() { JointAngles ja;ja.setDefaultPosition(); return ja; };

	JointAngles(const JointAngles& par) {
		for (int i = 0;i<NumberOfActuators;i++)
			a[i] = par.a[i];
	}

	void operator=(const JointAngles& par) {
		for (int i = 0;i<NumberOfActuators;i++)
			a[i] = par.a[i];
	}

	bool operator==(const JointAngles& par) {
		for (int i = 0;i<NumberOfActuators;i++)
			if (fabs(a[i]-par.a[i]) > floatPrecision)
				return false;
		return true;
	}

	bool operator!=(const JointAngles& pos) {
		return !((*this) == pos);
	};

	rational& operator[](int idx) {
		return a[idx];
	}
	const rational& operator[](int idx) const {
		return a[idx];
	}

	int size() const { return NumberOfActuators; };

	void null() {
		for (int i = 0;i<NumberOfActuators;i++)
			a[i] = 0.0;
	}
	bool isNull() {
		for (int i = 0;i<NumberOfActuators;i++)
			if (a[i] != 0.0)
				return false;
		return true;
	}


	void operator+=(const JointAngles& pos) {
		for (int i = 0;i<NumberOfActuators;i++)
			a[i] += pos.a[i];
	};

	void operator-=(const JointAngles& pos) {
		for (int i = 0;i<NumberOfActuators;i++)
			a[i] -= pos.a[i];
	};

	void operator*=(const float x) {
		for (int i = 0;i<NumberOfActuators;i++)
			a[i] *= x;
	};
	void operator/=(const float x) {
		for (int i = 0;i<NumberOfActuators;i++)
			a[i] /= x;
	};

	JointAngles operator*(float x) const {
		JointAngles result(*this);
		result *= x;
		return result;
	};

	JointAngles operator/(float x) const {
		JointAngles result(*this);
		result /= x;
		return result;
	};

	JointAngles  operator+(const JointAngles& pos) const {
		JointAngles result(*this);
		result += pos;
		return result;
	};
	JointAngles  operator-(const JointAngles& pos) const {
		JointAngles result(*this);
			result -= pos;
			return result;
		};

	string toString() const;
	bool fromString(const string& str, int& idx);

private:
	rational a[NumberOfActuators];
};


class Pose {
	public:
		friend ostream& operator<<(ostream&, const Pose&);

		Pose() {
			null();
		};
		Pose(const Pose& pose): Pose() {
			position = pose.position;
			orientation = pose.orientation;
			gripperAngle = pose.gripperAngle;
			angles = pose.angles;
		};
		Pose(const Point& pPosition, const Rotation& pOrientation, const rational pGripperAngle) {
			position = pPosition;
			orientation = pOrientation;
			gripperAngle = pGripperAngle;
			angles.null();
		};
		Pose(const Point& pPosition, const Rotation& pOrientation, const JointAngles& pAngles) {
			position = pPosition;
			orientation = pOrientation;
			gripperAngle = angles[GRIPPER];
			angles = pAngles;
		};

		void operator= (const Pose& pose) {
			position = pose.position;
			orientation = pose.orientation;
			gripperAngle = pose.gripperAngle;
			angles = pose.angles;
		}
		void null() {
			orientation.null();
			position.null();
			gripperAngle = 0.0;
			angles.null();
		}
		bool isNull() {
			return position.isNull();
		}

		void mirrorAt(const Pose& pMirror) {
			position.mirrorAt(pMirror.position);
			orientation.mirrorAt(pMirror.orientation);
			gripperAngle = pMirror.gripperAngle + (pMirror.gripperAngle-gripperAngle);
			angles = pMirror.angles + (pMirror.angles - angles);
		}

		float distance(const Pose& pPose) const {
			return sqrt((pPose.position[0]-position[0])*(pPose.position[0]-position[0]) +
						(pPose.position[1]-position[1])*(pPose.position[1]-position[1]) +
						(pPose.position[2]-position[2])*(pPose.position[2]-position[2]));
		}

		float length() const{
			return sqrt(sqr(position[0]) + sqr(position[1]) + sqr(position[2]));
		}

		bool operator==(const Pose& pPose) {
			return 	(position == pPose.position &&
					orientation == pPose.orientation) &&
					(gripperAngle == pPose.gripperAngle);

		};

		bool operator!=(const Pose& pos) {
			return !((*this) == pos);
		};

		void operator+=(const Pose& pos) {
			position += pos.position;
			for (int i = 0;i<3;i++)
				orientation[i] += pos.orientation[i];
			gripperAngle += pos.gripperAngle ;
		};
		void operator-=(const Pose& pos) {
			position -= pos.position;
			for (int i = 0;i<3;i++)
				orientation[i] -= pos.orientation[i];
			gripperAngle -= pos.gripperAngle ;
		};

		void operator*=(const float x) {
			position *= x;

			for (int i = 0;i<3;i++)
				orientation[i] *= x;
			gripperAngle = gripperAngle*x;

		};
		void operator/=(const float x) {
			position /= x;
			for (int i = 0;i<3;i++)
				orientation[i] /= x;
			gripperAngle = gripperAngle/x;
		};

		Pose operator*(float x) const {
			Pose result(*this);
			result *= x;
			return result;
		};

		Pose operator/(float x) const {
			Pose result(*this);
			result /= x;
			return result;
		};
		Pose  operator+(const Pose& pos) const {
			Pose result(*this);
			result += pos;
			return result;
		};

		Pose operator-(const Pose& pos) const {
			Pose result(*this);
			result -= pos;
			return result;
		};


		string toString() const;
		bool fromString(const string& str, int &idx);


	Point position;
	Rotation orientation;
	rational gripperAngle;

	JointAngles angles;
};


class TrajectoryNode {
public:

	 friend ostream& operator<<(ostream&, const TrajectoryNode&);

	TrajectoryNode() {
		null();
	}
	TrajectoryNode(const TrajectoryNode& par) {
		duration = par.duration;
		name = par.name;
		pose = par.pose;
		time = par.time;
		startSpeed = par.startSpeed;
		distance = par.distance;

		durationDef = par.durationDef;
		averageSpeedDef = par.averageSpeedDef;
		interpolationTypeDef = par.interpolationTypeDef;
	}
	void operator= (const TrajectoryNode& par) {
		duration = par.duration;
		name = par.name;
		pose = par.pose;
		time = par.time;
		startSpeed = par.startSpeed;
		distance = par.distance;

		averageSpeedDef = par.averageSpeedDef;
		durationDef = par.durationDef;
		interpolationTypeDef = par.interpolationTypeDef;
	}

	bool isPoseInterpolation() { return (!isJointInterpolation()); };
	bool isJointInterpolation() { return (interpolationTypeDef == JOINT_LINEAR); };

	string toString() const;
	bool fromString(const string& str, int &idx);


	string getText() const;
	bool isNull() {	return pose.isNull(); }
	void null() {
		duration = 0;
		interpolationTypeDef = POSE_CUBIC_BEZIER;
		time = 0;
		averageSpeedDef = 0;
		startSpeed = 0;
		distance = 0;
		durationDef = 0;
		pose.null();
		name.empty();
	}
	Pose pose;

	string name;							// some nodes have a name

	// the following attributes are available after compilation of a trajectory
	rational duration;  					// [ms] duration between this and next support node
	milliseconds time;						// absolute point in time of this support node (beginning from start of trajectory = 0)
	mmPerMillisecond startSpeed;			// current speed when this node is entered
	millimeter distance;					// distance to next node
	milliseconds minDuration;				// [ms] minimum time needed to move, computed by maximum speed of angle differences

	milliseconds durationDef;				// current duration that has been entered
	InterpolationType interpolationTypeDef;	// pose bezier, or poselinear, or joint interpolation
	mmPerMillisecond averageSpeedDef;		// average speed, use for input

};


#endif /* SPATIAL_H_ */