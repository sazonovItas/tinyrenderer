const std = @import("std");
const M44 = @import("math.zig").M44;
const Vec3f = @import("math.zig").Vec3f;
const math = @import("math.zig");

const UP = Vec3f.from(0.0, 1.0, 0.0);

pub fn SphereCamera() type {
    return struct {
        yaw: f32 = std.math.pi / 4.0,
        pitch: f32 = std.math.pi / 4.0,
        radius: f32 = 4.0,
        target: Vec3f,

        const Self = @This();

        pub fn from(target: Vec3f) Self {
            return Self{
                .target = target,
            };
        }

        pub fn position(self: Self) Vec3f {
            return Vec3f.from(
                std.math.cos(self.pitch) * std.math.cos(self.yaw) * self.radius,
                std.math.sin(self.pitch) * self.radius,
                std.math.cos(self.pitch) * std.math.sin(self.yaw) * self.radius,
            );
        }

        pub fn view(self: Self) M44 {
            return M44.lookat(self.position(), self.target, UP);
        }
    };
}
