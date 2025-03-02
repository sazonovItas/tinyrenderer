const std = @import("std");

pub const Vec2f = Vec2(f32);
pub const Vec3f = Vec3(f32);
pub const Vec4f = Vec4(f32);

pub const Vec2i = Vec2(i32);

pub inline fn abs(f: anytype) @TypeOf(f) {
    const type_info = @typeInfo(@TypeOf(f));
    if (type_info != .Float and type_info != .Int) {
        @compileError("expected integer or floating point type");
    }

    return if (f < 0) -f else f;
}

pub fn Vec2(comptime T: type) type {
    return struct {
        x: T,
        y: T,

        const Self = @This();

        pub inline fn from(x: T, y: T) Self {
            return Self{ .x = x, .y = y };
        }

        pub inline fn equal(self: Self, other: Self) bool {
            return self.x == other.x and self.y == other.y;
        }

        pub fn add(self: Self, other: Self) Self {
            return Self.from(self.x + other.x, self.y + other.y);
        }

        pub fn sub(self: Self, other: Self) Self {
            return Self.from(self.x - other.x, self.y - other.y);
        }

        pub fn scale(self: Self, factor: T) Self {
            return Self.from(self.x * factor, self.y * factor);
        }

        pub fn div(self: Self, division: T) Self {
            return Self.from(self.x / division, self.y / division);
        }

        pub fn scale_vec(self: Self, other: Self) Self {
            return Self.from(self.x * other.x, self.y * other.y);
        }

        pub fn dot(self: Self, other: Self) T {
            return self.x * other.x + self.y * other.y;
        }

        pub fn sqr_norm(self: Self) T {
            return self.x * self.x + self.y * self.y;
        }

        pub fn norm(self: Self) T {
            return @sqrt(self.sqr_norm());
        }

        pub fn perpendicular(self: Self) Self {
            return Self.from(self.y, self.x);
        }

        pub fn normalized(self: Self) Self {
            switch (@typeInfo(T)) {
                .Float => {
                    const magnitude = self.norm();
                    return Self.from(self.x / magnitude, self.y / magnitude);
                },
                else => @compileError("type " ++ @typeName(T) ++ "is not floating point number"),
            }
        }

        pub fn to_vec3(self: Self, z: T) Vec3(T) {
            return Vec3(T).from(self.x, self.y, z);
        }
    };
}

pub fn Vec3(comptime T: type) type {
    return struct {
        x: T,
        y: T,
        z: T,

        const Self = @This();

        pub inline fn from(x: T, y: T, z: T) Self {
            return Self{ .x = x, .y = y, .z = z };
        }

        pub inline fn equal(self: Self, other: Self) bool {
            return self.x == other.x and self.y == other.y and self.z == other.z;
        }

        pub fn add(self: Self, other: Self) Self {
            return Self.from(self.x + other.x, self.y + other.y, self.z + other.z);
        }

        pub fn sub(self: Self, other: Self) Self {
            return Self.from(self.x - other.x, self.y - other.y, self.z - other.z);
        }

        pub fn scale(self: Self, factor: T) Self {
            return Self.from(self.x * factor, self.y * factor, self.z * factor);
        }

        pub fn div(self: Self, division: T) Self {
            return Self.from(self.x / division, self.y / division, self.z / division);
        }

        pub fn scale_vec(self: Self, other: Self) Self {
            return Self.from(self.x * other.x, self.y * other.y, self.z * other.z);
        }

        pub fn dot(self: Self, other: Self) T {
            return self.x * other.x + self.y * other.y + self.z * other.z;
        }

        pub fn cross_product(self: Self, other: Self) Self {
            return Self.from(
                self.y * other.z - self.z * other.y,
                self.z * other.x - self.x * other.z,
                self.x * other.y - self.y * other.x,
            );
        }

        pub fn sqr_norm(self: Self) T {
            return self.x * self.x + self.y * self.y + self.z * self.z;
        }

        pub fn norm(self: Self) T {
            return @sqrt(self.sqr_norm());
        }

        pub fn normalized(self: Self) Self {
            switch (@typeInfo(T)) {
                .Float => {
                    const magnitude = self.norm();
                    return Self.from(self.x / magnitude, self.y / magnitude, self.z / magnitude);
                },
                else => @compileError("type " ++ @typeName(T) ++ "is not floating point number"),
            }
        }

        pub fn to_vec4(self: Self, w: T) Vec4(T) {
            return Vec4(T).from(self.x, self.y, self.z, w);
        }
    };
}

pub fn Vec4(comptime T: type) type {
    return struct {
        x: T,
        y: T,
        z: T,
        w: T,

        const Self = @This();

        pub inline fn from(x: T, y: T, z: T, w: T) Self {
            return Self{ .x = x, .y = y, .z = z, .w = w };
        }

        pub inline fn equal(self: Self, other: Self) bool {
            return self.x == other.x and self.y == other.y and self.z == other.z and self.w == self.w;
        }

        pub fn add(self: Self, other: Self) Self {
            return Self.from(
                self.x + other.x,
                self.y + other.y,
                self.z + other.z,
                if (self.w > 0 and self.w > 0) 1.0 else 0.0,
            );
        }

        pub fn scale(self: Self, factor: T) Self {
            return Self.from(self.x * factor, self.y * factor, self.z * factor, self.w * factor);
        }

        pub fn div(self: Self, division: T) Self {
            return Self.from(self.x / division, self.y / division, self.z / division, self.w / division);
        }

        pub fn scale_vec(self: Self, other: Self) Self {
            return Self.from(self.x * other.x, self.y * other.y, self.z * other.z, self.w * other.w);
        }

        pub fn dot(self: Self, other: Self) T {
            return self.x * other.x + self.y * other.y + self.z * other.z;
        }

        pub fn perspective_div(self: Self) Self {
            return Self.from(self.x / self.w, self.y / self.w, self.z / self.w, self.w / self.w);
        }
    };
}

pub const M44 = struct {
    i: Vec4f,
    j: Vec4f,
    k: Vec4f,
    t: Vec4f,

    const Self = @This();

    pub inline fn from(i: Vec4f, j: Vec4f, k: Vec4f, t: Vec4f) Self {
        return Self{ .i = i, .j = j, .k = k, .t = t };
    }

    pub inline fn identity() Self {
        return Self.from(
            Vec4f.from(1.0, 0.0, 0.0, 0.0),
            Vec4f.from(0.0, 1.0, 0.0, 0.0),
            Vec4f.from(0.0, 0.0, 1.0, 0.0),
            Vec4f.from(0.0, 0.0, 0.0, 1.0),
        );
    }

    pub inline fn scale_mat(v: Vec3f) Self {
        return Self.from(
            Vec4f.from(v.x, 0.0, 0.0, 0.0),
            Vec4f.from(0.0, v.y, 0.0, 0.0),
            Vec4f.from(0.0, 0.0, v.z, 0.0),
            Vec4f.from(0.0, 0.0, 0.0, 1.0),
        );
    }

    pub fn scale(self: Self, v: Vec3f) Self {
        return Self.scale_mat(v).mul(self);
    }

    pub fn rotate_mat(axis: Vec3f, angle_rad: f32) Self {
        const c = @cos(angle_rad);
        const s = @sin(angle_rad);
        const t = 1.0 - c;

        const sqr_norm = axis.sqr_norm();
        if (sqr_norm == 0.0) {
            return Self.identity();
        } else if (abs(sqr_norm - 1.0) > 0.0001) {
            return rotate_mat(axis.normalized(), angle_rad);
        }

        const x = axis.x;
        const y = axis.y;
        const z = axis.z;

        return Self.from(
            Vec4f.from(x * x * t + c, y * x * t + z * s, z * x * t - y * s, 0.0),
            Vec4f.from(x * y * t - z * s, y * y * t + c, z * y * t + x * s, 0.0),
            Vec4f.from(x * z * t + y * s, y * z * t - x * s, z * z * t + c, 0.0),
            Vec4f.from(0.0, 0.0, 0.0, 1.0),
        );
    }

    pub fn rotate(self: Self, v: Vec3f, angle_rad: f32) Self {
        return Self.rotate_mat(v, angle_rad).mul(self);
    }

    pub inline fn translate_mat(v: Vec3f) Self {
        return Self.from(
            Vec4f.from(1.0, 0.0, 0.0, v.x),
            Vec4f.from(0.0, 1.0, 0.0, v.y),
            Vec4f.from(0.0, 0.0, 1.0, v.z),
            Vec4f.from(0.0, 0.0, 0.0, 1.0),
        );
    }

    pub fn translate(self: Self, v: Vec3f) Self {
        return Self.translate_mat(v).mul(self);
    }

    pub inline fn scale_by_factor_mat(factor: f32) Self {
        return Self.from(
            Vec4f.from(factor, 0.0, 0.0, 0.0),
            Vec4f.from(0.0, factor, 0.0, 0.0),
            Vec4f.from(0.0, 0.0, factor, 0.0),
            Vec4f.from(0.0, 0.0, 0.0, 1.0),
        );
    }

    pub fn scale_by_factor(self: Self, factor: f32) Self {
        return Self.scale_by_factor_mat(factor).mul(self);
    }

    pub inline fn transpose(self: Self) Self {
        return Self.from(
            Vec4f.from(self.i.x, self.j.x, self.k.x, self.t.x),
            Vec4f.from(self.i.y, self.j.y, self.k.y, self.t.y),
            Vec4f.from(self.i.z, self.j.z, self.k.z, self.t.z),
            Vec4f.from(self.i.w, self.j.w, self.k.w, self.t.w),
        );
    }

    pub fn mul(a: Self, b: Self) Self {
        return Self.from(
            Vec4f.from(
                a.i.x * b.i.x + a.i.y * b.j.x + a.i.z * b.k.x + a.i.w * b.t.x,
                a.i.x * b.i.y + a.i.y * b.j.y + a.i.z * b.k.y + a.i.w * b.t.y,
                a.i.x * b.i.z + a.i.y * b.j.z + a.i.z * b.k.z + a.i.w * b.t.z,
                a.i.x * b.i.w + a.i.y * b.j.w + a.i.z * b.k.w + a.i.w * b.t.w,
            ),
            Vec4f.from(
                a.j.x * b.i.x + a.j.y * b.j.x + a.j.z * b.k.x + a.j.w * b.t.x,
                a.j.x * b.i.y + a.j.y * b.j.y + a.j.z * b.k.y + a.j.w * b.t.y,
                a.j.x * b.i.z + a.j.y * b.j.z + a.j.z * b.k.z + a.j.w * b.t.z,
                a.j.x * b.i.w + a.j.y * b.j.w + a.j.z * b.k.w + a.j.w * b.t.w,
            ),
            Vec4f.from(
                a.k.x * b.i.x + a.k.y * b.j.x + a.k.z * b.k.x + a.k.w * b.t.x,
                a.k.x * b.i.y + a.k.y * b.j.y + a.k.z * b.k.y + a.k.w * b.t.y,
                a.k.x * b.i.z + a.k.y * b.j.z + a.k.z * b.k.z + a.k.w * b.t.z,
                a.k.x * b.i.w + a.k.y * b.j.w + a.k.z * b.k.w + a.k.w * b.t.w,
            ),
            Vec4f.from(
                a.t.x * b.i.x + a.t.y * b.j.x + a.t.z * b.k.x + a.t.w * b.t.x,
                a.t.x * b.i.y + a.t.y * b.j.y + a.t.z * b.k.y + a.t.w * b.t.y,
                a.t.x * b.i.z + a.t.y * b.j.z + a.t.z * b.k.z + a.t.w * b.t.z,
                a.t.x * b.i.w + a.t.y * b.j.w + a.t.z * b.k.w + a.t.w * b.t.w,
            ),
        );
    }

    pub fn lookat(camera: Vec3f, target: Vec3f, up: Vec3f) Self {
        const normalized_up = up.normalized();

        const new_forward: Vec3f = camera.sub(target).normalized();
        const new_right: Vec3f = normalized_up.cross_product(new_forward).normalized();
        const new_up: Vec3f = new_forward.cross_product(new_right).normalized();

        const dot_right: f32 = new_right.dot(camera);
        const dot_up: f32 = new_up.dot(camera);
        const dot_forward: f32 = new_forward.dot(camera);

        return Self.from(
            Vec4f.from(new_right.x, new_right.y, new_right.z, -dot_right),
            Vec4f.from(new_up.x, new_up.y, new_up.z, -dot_up),
            Vec4f.from(new_forward.x, new_forward.y, new_forward.z, -dot_forward),
            Vec4f.from(0.0, 0.0, 0.0, 1.0),
        );
    }

    pub fn orto(width: f32, height: f32, z_near: f32, z_far: f32) Self {
        return Self.from(
            Vec4f.from(2.0 / width, 0.0, 0.0, 0.0),
            Vec4f.from(0.0, 2.0 / height, 0.0, 0.0),
            Vec4f.from(0.0, 0.0, 1 / (z_near - z_far), z_near / (z_near - z_far)),
            Vec4f.from(0.0, 0.0, 0.0, 1.0),
        );
    }

    pub fn perspective(fov: f32, aspect: f32, z_near: f32, z_far: f32) Self {
        const t = std.math.tan(fov / 2.0);

        return Self.from(
            Vec4f.from(1 / (aspect * t), 0.0, 0.0, 0.0),
            Vec4f.from(0.0, 1.0 / t, 0.0, 0.0),
            Vec4f.from(0.0, 0.0, z_far / (z_near - z_far), (z_near * z_far) / (z_near - z_far)),
            Vec4f.from(0.0, 0.0, -1.0, 0.0),
        );
    }

    pub fn viewport(x_int: i32, y_int: i32, w_int: i32, h_int: i32, d: f32) Self {
        const x: f32 = @floatFromInt(x_int);
        const y: f32 = @floatFromInt(y_int);
        const w: f32 = @floatFromInt(w_int);
        const h: f32 = @floatFromInt(h_int);

        return Self.from(
            Vec4f.from(w / 2.0, 0.0, 0.0, x + w / 2.0),
            Vec4f.from(0.0, -h / 2.0, 0.0, y + h / 2.0),
            Vec4f.from(0.0, 0.0, d / 2.0, d / 2.0),
            Vec4f.from(0.0, 0.0, 0.0, 1.0),
        );
    }

    pub fn apply_to_point(self: Self, point: Vec3f) Vec3f {
        const v = self.apply_to_vec4(Vec4f.from(point.x, point.y, point.z, 1.0));
        return Vec3f.from(v.x, v.y, v.z);
    }

    pub fn apply_to_vector(self: Self, vector: Vec3f) Vec3f {
        const v = self.apply_to_vec4(Vec4f.from(vector.x, vector.y, vector.z, 0.0));
        return Vec3f.from(v.x, v.y, v.z);
    }

    pub fn apply_to_vec4(self: Self, v: Vec4f) Vec4f {
        return Vec4f.from(
            self.i.x * v.x + self.i.y * v.y + self.i.z * v.z + self.i.w * v.w,
            self.j.x * v.x + self.j.y * v.y + self.j.z * v.z + self.j.w * v.w,
            self.k.x * v.x + self.k.y * v.y + self.k.z * v.z + self.k.w * v.w,
            self.t.x * v.x + self.t.y * v.y + self.t.z * v.z + self.t.w * v.w,
        );
    }
};

fn map_range_to_range(from: f32, to: f32, map_from: f32, map_to: f32) f32 {
    return (map_to - map_from) / (to - from);
}

fn map_range_to_range_normalized(n: f32, from: f32, to: f32, map_from: f32, map_to: f32) f32 {
    const factor = ((map_to - map_from) / (to - from));
    return ((n - from) * factor) + map_from;
}

pub inline fn radians(degrs: f32) f32 {
    return (@as(f32, std.math.pi) / 180.0) * degrs;
}

pub inline fn degrees(rads: f32) f32 {
    return (180.0 / @as(f32, std.math.pi)) * rads;
}
