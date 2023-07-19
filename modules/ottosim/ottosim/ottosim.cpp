#include <Python.h>
#include <structmember.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Featherstone/btMultiBodyConstraintSolver.h>
#include <BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h>
#include <BulletDynamics/Featherstone/btMultiBodyJointLimitConstraint.h>
#include <BulletDynamics/Featherstone/btMultiBodyJointMotor.h>
#include <BulletDynamics/Featherstone/btMultiBodyLinkCollider.h>

#include <random>
#include <chrono>

std::mt19937_64 rng = std::mt19937_64(std::chrono::high_resolution_clock::now().time_since_epoch().count());

struct Random {
    std::mt19937_64 rng;
    std::normal_distribution<double> dist;
    Random(unsigned long long seed) {
        rng = std::mt19937_64(seed);
        dist = std::normal_distribution<double>(0.0, 1.0);
    }
    double get() {
        return dist(rng);
    }
};

struct OttoEnv {
    PyObject_HEAD

    int ready;
    double time;

    struct {
        bool low_friction;
    } settings;

    btMultiBodyDynamicsWorld * world;
    btRigidBody * ground;
    btMultiBody * robot;

    btMultiBodyLinkCollider * head;
    btMultiBodyLinkCollider * leg_L;
    btMultiBodyLinkCollider * foot_L;
    btMultiBodyLinkCollider * leg_R;
    btMultiBodyLinkCollider * foot_R;

    btMultiBodyJointLimitConstraint * leg_L_limit;
    btMultiBodyJointLimitConstraint * foot_L_limit;
    btMultiBodyJointLimitConstraint * leg_R_limit;
    btMultiBodyJointLimitConstraint * foot_R_limit;

    btMultiBodyJointMotor * leg_L_motor;
    btMultiBodyJointMotor * foot_L_motor;
    btMultiBodyJointMotor * leg_R_motor;
    btMultiBodyJointMotor * foot_R_motor;

    Random * noise;
};

static PyTypeObject * OttoEnv_type;

const double pi = 3.1415926535;
const double dt = 1.0 / 300.0;

btBoxShape * box(double x, double y, double z) {
    btBoxShape * shape = new btBoxShape({x * 0.5, y * 0.5, z * 0.5});
    shape->setMargin(0.0001);
    return shape;
}

btVector3 inertia(double mass, btCollisionShape * shape) {
    btVector3 inertia = {0.0, 0.0, 0.0};
    shape->calculateLocalInertia(mass, inertia);
    return inertia;
}

void OttoEnv_init(OttoEnv * self, unsigned long long seed) {
    btDefaultCollisionConfiguration * configuration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher * dispatcher = new btCollisionDispatcher(configuration);
    btDbvtBroadphase * broadphase = new btDbvtBroadphase();
    btMultiBodyConstraintSolver * solver = new btMultiBodyConstraintSolver();
    self->world = new btMultiBodyDynamicsWorld(dispatcher, broadphase, solver, configuration);

    self->noise = new Random(seed);
    solver->setRandSeed((unsigned)(seed & seed >> 32));

    self->world->getSolverInfo().m_solverMode |= SOLVER_RANDMIZE_ORDER;
    self->world->setGravity({self->noise->get() * 0.01, self->noise->get() * 0.01, -9.8});

    self->ground = new btRigidBody(0.0, NULL, box(100.0, 100.0, 100.0), {0.0, 0.0, 0.0});
    self->ground->setWorldTransform(btTransform({0.0, 0.0, 0.0, 1.0}, {0.0, 0.0, -50.0}));

    self->world->addRigidBody(self->ground, 1, 2);

    btCollisionShape * head_shape = box(0.07, 0.07, 0.07);
    btCollisionShape * leg_L_shape = box(0.018, 0.035, 0.0415);
    btCollisionShape * foot_L_shape = box(0.045, 0.06, 0.005);
    btCollisionShape * leg_R_shape = box(0.018, 0.035, 0.0415);
    btCollisionShape * foot_R_shape = box(0.045, 0.06, 0.005);

    self->robot = new btMultiBody(4, 0.16, inertia(0.16, head_shape), false, false, false);
    self->robot->setupRevolute(0, 0.02, inertia(0.02, leg_L_shape), -1, {0.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 1.0}, {0.027, 0.0, -0.031}, {0.0, 0.0, -0.02}, false);
    self->robot->setupRevolute(1, 0.008, inertia(0.008, foot_L_shape), 0, {0.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, -0.01}, {0.009, 0.0, -0.0165}, false);
    self->robot->setupRevolute(2, 0.02, inertia(0.02, leg_R_shape), -1, {0.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 1.0}, {-0.027, 0.0, -0.031}, {0.0, 0.0, -0.02}, false);
    self->robot->setupRevolute(3, 0.008, inertia(0.008, foot_R_shape), 2, {0.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, -0.01}, {-0.009, 0.0, -0.0165}, false);
    self->robot->finalizeMultiDof();

    self->leg_L_limit = new btMultiBodyJointLimitConstraint(self->robot, 0, -pi / 2.0, pi / 2.0);
    self->foot_L_limit = new btMultiBodyJointLimitConstraint(self->robot, 1, -pi / 2.0, pi / 2.0);
    self->leg_R_limit = new btMultiBodyJointLimitConstraint(self->robot, 2, -pi / 2.0, pi / 2.0);
    self->foot_R_limit = new btMultiBodyJointLimitConstraint(self->robot, 3, -pi / 2.0, pi / 2.0);

    self->leg_L_motor = new btMultiBodyJointMotor(self->robot, 0, 0.0, 0.001);
    self->foot_L_motor = new btMultiBodyJointMotor(self->robot, 1, 0.0, 0.001);
    self->leg_R_motor = new btMultiBodyJointMotor(self->robot, 2, 0.0, 0.001);
    self->foot_R_motor = new btMultiBodyJointMotor(self->robot, 3, 0.0, 0.001);

    double base_rotation = self->noise->get() * 0.1;
    btVector3 base_position = {self->noise->get() * 0.01, self->noise->get() * 0.01, 0.0};
    btTransform base_transform = btTransform({0.0, 0.0, sin(base_rotation * 0.5), cos(base_rotation * 0.5)}, base_position);

    btTransform head_transform = base_transform * btTransform({0.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 0.083});
    btTransform leg_L_transform = base_transform * btTransform({0.0, 0.0, 0.0, 1.0}, {0.027, 0.0, 0.034});
    btTransform foot_L_transform = base_transform * btTransform({0.0, 0.0, 0.0, 1.0}, {0.036, 0.0, 0.0025});
    btTransform leg_R_transform = base_transform * btTransform({0.0, 0.0, 0.0, 1.0}, {-0.027, 0.0, 0.034});
    btTransform foot_R_transform = base_transform * btTransform({0.0, 0.0, 0.0, 1.0}, {-0.036, 0.0, 0.0025});

    self->head = new btMultiBodyLinkCollider(self->robot, -1);
    self->head->setCollisionShape(head_shape);
    self->head->setWorldTransform(head_transform);

    self->leg_L = new btMultiBodyLinkCollider(self->robot, 0);
    self->leg_L->setCollisionShape(leg_L_shape);
    self->leg_L->setWorldTransform(leg_L_transform);

    self->foot_L = new btMultiBodyLinkCollider(self->robot, 1);
    self->foot_L->setCollisionShape(foot_L_shape);
    self->foot_L->setWorldTransform(foot_L_transform);

    self->leg_R = new btMultiBodyLinkCollider(self->robot, 2);
    self->leg_R->setCollisionShape(leg_R_shape);
    self->leg_R->setWorldTransform(leg_R_transform);

    self->foot_R = new btMultiBodyLinkCollider(self->robot, 3);
    self->foot_R->setCollisionShape(foot_R_shape);
    self->foot_R->setWorldTransform(foot_R_transform);

    self->robot->setBaseWorldTransform(head_transform);
    self->robot->setBaseCollider(self->head);

    self->robot->getLink(0).m_collider = self->leg_L;
    self->robot->getLink(1).m_collider = self->foot_L;
    self->robot->getLink(2).m_collider = self->leg_R;
    self->robot->getLink(3).m_collider = self->foot_R;

    self->world->addCollisionObject(self->head, 2, 1);
    self->world->addCollisionObject(self->leg_L, 2, 1);
    self->world->addCollisionObject(self->foot_L, 2, 1);
    self->world->addCollisionObject(self->leg_R, 2, 1);
    self->world->addCollisionObject(self->foot_R, 2, 1);
    self->world->addMultiBody(self->robot, 2, 1);

    self->world->addMultiBodyConstraint(self->leg_L_limit);
    self->world->addMultiBodyConstraint(self->foot_L_limit);
    self->world->addMultiBodyConstraint(self->leg_R_limit);
    self->world->addMultiBodyConstraint(self->foot_R_limit);

    self->world->addMultiBodyConstraint(self->leg_L_motor);
    self->world->addMultiBodyConstraint(self->foot_L_motor);
    self->world->addMultiBodyConstraint(self->leg_R_motor);
    self->world->addMultiBodyConstraint(self->foot_R_motor);

    self->leg_L_limit->finalizeMultiDof();
    self->foot_L_limit->finalizeMultiDof();
    self->leg_R_limit->finalizeMultiDof();
    self->foot_R_limit->finalizeMultiDof();

    self->leg_L_motor->finalizeMultiDof();
    self->foot_L_motor->finalizeMultiDof();
    self->leg_R_motor->finalizeMultiDof();
    self->foot_R_motor->finalizeMultiDof();

    self->ground->setFriction(0.75);
    self->ground->setRollingFriction(0.01);
    self->ground->setSpinningFriction(0.01);

    self->foot_L->setFriction(0.75);
    self->foot_L->setRollingFriction(0.01);
    self->foot_L->setSpinningFriction(0.01);

    self->foot_R->setFriction(0.75);
    self->foot_R->setRollingFriction(0.01);
    self->foot_R->setSpinningFriction(0.01);

    if (self->settings.low_friction) {
        self->ground->setFriction(0.1);
        self->foot_L->setFriction(0.1);
        self->foot_R->setFriction(0.1);
    }

    for (int i = 0; i < 10; ++i) {
        self->leg_L_motor->setPositionTarget(0.0);
        self->foot_L_motor->setPositionTarget(0.0);
        self->leg_R_motor->setPositionTarget(0.0);
        self->foot_R_motor->setPositionTarget(0.0);
        self->world->stepSimulation(dt, 0, dt);
    }

    self->time = 0.0;
    self->ready = true;
}

void OttoEnv_release(OttoEnv * self) {
    // TODO: implement
    self->ready = false;
}

static OttoEnv * OttoEnv_new() {
    OttoEnv * res = PyObject_New(OttoEnv, OttoEnv_type);
    res->settings = {};
    res->ready = false;
    return res;
}

static OttoEnv * meth_make(PyObject * self, PyObject * args, PyObject * kwargs) {
    const char * keywords[] = {"id", NULL};

    const char * id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", (char **)keywords, &id)) {
        return NULL;
    }

    if (!strcmp(id, "Otto-v0")) {
        return OttoEnv_new();
    }

    if (!strcmp(id, "OttoLowFriction-v0")) {
        OttoEnv * env = OttoEnv_new();
        env->settings.low_friction = true;
        return env;
    }

    PyErr_Format(PyExc_ValueError, "env not found");
    return NULL;
}

static PyObject * OttoEnv_observation(OttoEnv * self) {
    return Py_BuildValue("[d]", 0.0);
}

static PyObject * OttoEnv_info(OttoEnv * self) {
    return Py_BuildValue("{}");
}

static PyObject * OttoEnv_meth_reset(OttoEnv * self, PyObject * args, PyObject * kwargs) {
    const char * keywords[] = {"seed", NULL};

    PyObject * seed_arg = Py_None;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O", (char **)keywords, &seed_arg)) {
        return NULL;
    }

    unsigned seed = rng();
    if (seed_arg != Py_None) {
        seed = PyLong_AsUnsignedLong(seed_arg);
        if (PyErr_Occurred()) {
            PyErr_Format(PyExc_ValueError, "invalid seed");
            return NULL;
        }
    }

    if (self->ready) {
        OttoEnv_release(self);
    }

    OttoEnv_init(self, seed);
    return Py_BuildValue("(NN)", OttoEnv_observation(self), OttoEnv_info(self));
}

static PyObject * OttoEnv_meth_step(OttoEnv * self, PyObject * args, PyObject * kwargs) {
    const char * keywords[] = {"action", NULL};

    PyObject * action_arg;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", (char **)keywords, &action_arg)) {
        return NULL;
    }

    if (!self->ready) {
        PyErr_Format(PyExc_RuntimeError, "reset was not called");
        return NULL;
    }

    PyObject * seq = PySequence_Fast(action_arg, "");
    if (!seq || PySequence_Fast_GET_SIZE(seq) != 4) {
        PyErr_Format(PyExc_ValueError, "invalid action");
        return NULL;
    }
    double action[4] = {};
    action[0] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(seq, 0));
    action[1] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(seq, 1));
    action[2] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(seq, 2));
    action[3] = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(seq, 3));
    if (PyErr_Occurred()) {
        PyErr_Format(PyExc_ValueError, "invalid action");
        return NULL;
    }
    Py_DECREF(seq);
    self->leg_L_motor->setPositionTarget(action[0]);
    self->foot_L_motor->setPositionTarget(action[1]);
    self->leg_R_motor->setPositionTarget(action[2]);
    self->foot_R_motor->setPositionTarget(action[3]);
    self->world->stepSimulation(dt, 0, dt);
    self->time += dt;
    return Py_BuildValue("(NdOON)", OttoEnv_observation(self), 0.0, Py_False, Py_False, OttoEnv_info(self));
}

static PyObject * OttoEnv_meth_bones(OttoEnv * self, PyObject * args) {
    if (!self->ready) {
        PyErr_Format(PyExc_RuntimeError, "reset was not called");
        return NULL;
    }

    PyObject * res = PyBytes_FromStringAndSize(NULL, 5 * 7 * 4);
    float * ptr = (float *)PyBytes_AsString(res);
    btCollisionObject * bodies[] = {self->head, self->leg_L, self->foot_L, self->leg_R, self->foot_R};
    for (int i = 0; i < 5; ++i) {
        btTransform t = bodies[i]->getWorldTransform();
        btVector3 p = t.getOrigin();
        btQuaternion r = t.getRotation();
        *ptr++ = (float)p.x();
        *ptr++ = (float)p.y();
        *ptr++ = (float)p.z();
        *ptr++ = (float)r.x();
        *ptr++ = (float)r.y();
        *ptr++ = (float)r.z();
        *ptr++ = (float)r.w();
    }
    return res;
}

static void OttoEnv_dealloc(OttoEnv * self) {
    if (self->ready) {
        OttoEnv_release(self);
    }
    Py_TYPE(self)->tp_free(self);
}

static PyMethodDef OttoEnv_methods[] = {
    {"reset", (PyCFunction)OttoEnv_meth_reset, METH_VARARGS | METH_KEYWORDS},
    {"step", (PyCFunction)OttoEnv_meth_step, METH_VARARGS | METH_KEYWORDS},
    {"bones", (PyCFunction)OttoEnv_meth_bones, METH_NOARGS},
    {},
};

static PyMemberDef OttoEnv_members[] = {
    {"time", T_DOUBLE, offsetof(OttoEnv, time), READONLY},
    {"ready", T_BOOL, offsetof(OttoEnv, ready), READONLY},
    {},
};

static PyType_Slot OttoEnv_slots[] = {
    {Py_tp_methods, OttoEnv_methods},
    {Py_tp_members, OttoEnv_members},
    {Py_tp_dealloc, (void *)OttoEnv_dealloc},
    {},
};

static PyType_Spec OttoEnv_spec = {"OttoEnv", sizeof(OttoEnv), 0, Py_TPFLAGS_DEFAULT, OttoEnv_slots};

static PyMethodDef module_methods[] = {
    {"make", (PyCFunction)meth_make, METH_VARARGS | METH_KEYWORDS},
    {},
};

static PyModuleDef module_def = {PyModuleDef_HEAD_INIT, "ottosim", NULL, -1, module_methods};

extern "C" PyObject * PyInit_ottosim() {
    PyObject * module = PyModule_Create(&module_def);
    OttoEnv_type = (PyTypeObject *)PyType_FromSpec(&OttoEnv_spec);
    PyModule_AddObject(module, "OttoEnv", (PyObject *)OttoEnv_type);
    return module;
}
